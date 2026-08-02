#include "VersionInfo.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

VersionInfo::VersionInfo(QObject *parent)
    : QObject(parent)
{
    load();
}

void VersionInfo::load()
{
    QFile f(QStringLiteral(":/gitdesk/resources/version.json"));
    if (!f.open(QIODevice::ReadOnly)) {
#ifdef GITDESK_VERSION
        m_version = QStringLiteral(GITDESK_VERSION);
#endif
        m_summary = QStringLiteral("现代化 Git 可视化工作台");
        m_tagline = m_summary;
        m_description = m_summary;
        emit changed();
        return;
    }

    QJsonParseError err{};
    const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning("version.json parse error: %s", qPrintable(err.errorString()));
        return;
    }

    const QJsonObject o = doc.object();
    m_name = o.value(QStringLiteral("name")).toString(m_name);
    m_version = o.value(QStringLiteral("version")).toString(m_version);
    m_buildDate = o.value(QStringLiteral("buildDate")).toString();
    m_channel = o.value(QStringLiteral("channel")).toString(m_channel);
    m_organization = o.value(QStringLiteral("organization")).toString(m_organization);
    m_author = o.value(QStringLiteral("author")).toString(m_author);
    m_summary = o.value(QStringLiteral("summary")).toString();

    const QJsonObject about = o.value(QStringLiteral("about")).toObject();
    m_tagline = about.value(QStringLiteral("tagline")).toString(m_summary);
    m_description = about.value(QStringLiteral("description")).toString(m_summary);
    m_highlights.clear();
    for (const auto &v : about.value(QStringLiteral("highlights")).toArray())
        m_highlights << v.toString();

    m_changelog.clear();
    for (const auto &entryVal : o.value(QStringLiteral("changelog")).toArray()) {
        const QJsonObject e = entryVal.toObject();
        QVariantMap entry;
        entry.insert(QStringLiteral("version"), e.value(QStringLiteral("version")).toString());
        entry.insert(QStringLiteral("date"), e.value(QStringLiteral("date")).toString());
        entry.insert(QStringLiteral("title"), e.value(QStringLiteral("title")).toString());
        QVariantList changes;
        for (const auto &cVal : e.value(QStringLiteral("changes")).toArray()) {
            const QJsonObject c = cVal.toObject();
            QVariantMap cm;
            cm.insert(QStringLiteral("type"), c.value(QStringLiteral("type")).toString());
            cm.insert(QStringLiteral("text"), c.value(QStringLiteral("text")).toString());
            changes.push_back(cm);
        }
        entry.insert(QStringLiteral("changes"), changes);
        m_changelog.push_back(entry);
    }

    emit changed();
}

QString VersionInfo::aboutPlainText() const
{
    QStringList lines;
    lines << m_name + QLatin1Char(' ') + m_version;
    if (!m_tagline.isEmpty())
        lines << m_tagline;
    if (!m_description.isEmpty())
        lines << QString() << m_description;
    if (!m_author.isEmpty())
        lines << QString() << QObject::tr("作者：%1").arg(m_author);
    return lines.join(QLatin1Char('\n'));
}
