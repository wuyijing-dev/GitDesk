#include "LocaleController.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QQmlEngine>

LocaleController::LocaleController(QObject *parent)
    : QObject(parent)
{
}

void LocaleController::setEngine(QQmlEngine *engine)
{
    m_engine = engine;
}

QStringList LocaleController::availableLanguages() const
{
    return { QStringLiteral("zh-CN"), QStringLiteral("en-US") };
}

QString LocaleController::normalize(const QString &languageCode) const
{
    const QString raw = languageCode.trimmed();
    if (raw.isEmpty())
        return QStringLiteral("zh-CN");
    if (raw.startsWith(QLatin1String("en"), Qt::CaseInsensitive))
        return QStringLiteral("en-US");
    if (raw.startsWith(QLatin1String("zh"), Qt::CaseInsensitive))
        return QStringLiteral("zh-CN");
    return raw;
}

QString LocaleController::toMd3Language(const QString &languageCode) const
{
    return normalize(languageCode) == QLatin1String("en-US")
               ? QStringLiteral("en")
               : QStringLiteral("zh_CN");
}

QString LocaleController::displayName(const QString &languageCode) const
{
    const QString n = normalize(languageCode);
    if (n == QLatin1String("en-US"))
        return QStringLiteral("English");
    return QStringLiteral("简体中文");
}

bool LocaleController::installAppTranslator(const QString &normalized)
{
    QCoreApplication *app = QCoreApplication::instance();
    if (!app)
        return false;

    app->removeTranslator(&m_translator);

    if (normalized == QLatin1String("zh-CN"))
        return true;

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QStringLiteral(":/gitdesk/i18n/gitdesk_en.qm"),
        appDir + QStringLiteral("/i18n/gitdesk_en.qm"),
        appDir + QStringLiteral("/translations/gitdesk_en.qm"),
    };

    for (const QString &path : candidates) {
        // QFile::exists works for :/ resources too
        if (!QFile::exists(path))
            continue;
        if (m_translator.load(path)) {
            app->installTranslator(&m_translator);
            return true;
        }
        qWarning("LocaleController: failed to load %s", qPrintable(path));
    }

    qWarning("LocaleController: gitdesk_en.qm not found — English UI strings stay Chinese source");
    return false;
}

bool LocaleController::apply(const QString &languageCode)
{
    const QString normalized = normalize(languageCode);
    const bool ok = installAppTranslator(normalized);
    const bool changed = (m_language != normalized);
    m_language = normalized;
    ++m_revision;

    QLocale::setDefault(QLocale(normalized == QLatin1String("en-US")
                                    ? QStringLiteral("en_US")
                                    : QStringLiteral("zh_CN")));

    if (m_engine) {
        // Prefer BCP-47 tags that match Qt Quick translation lookup
        m_engine->setUiLanguage(normalized == QLatin1String("en-US")
                                    ? QStringLiteral("en_US")
                                    : QStringLiteral("zh_CN"));
        m_engine->retranslate();
    }

    emit languageChanged(); // always — drives QML bindings that depend on revision
    if (!ok && normalized == QLatin1String("en-US"))
        qWarning("LocaleController: English pack missing; UI may stay Chinese");
    Q_UNUSED(changed)
    return true;
}
