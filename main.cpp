#include "md3.h"

#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QFile>
#include <QUrl>
#include <QDebug>

#if defined(MD3_SHARED)
#  include <QPluginLoader>
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
#  include <QApplication>
#else
#  include <QGuiApplication>
#endif

#if !defined(MD3_SHARED)
#  include <QtQml/qqmlextensionplugin.h>
Q_IMPORT_QML_PLUGIN(Md3Plugin)
#endif

int main(int argc, char *argv[])
{
    Md3::RunOptions opts;
    opts.organization = QStringLiteral("wuyijing-dev");
    opts.applicationName = QStringLiteral("GitDesk");
    opts.applicationVersion = QStringLiteral(GITDESK_VERSION);
    opts.style = QStringLiteral("Basic");
    opts.loadFonts = true;
    opts.desktopFileName = QStringLiteral("GitDesk");
    opts.appUserModelId = QStringLiteral("GitDesk.Git.Workbench");

    Md3::applyEarly(argc, argv, opts);

#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
    QApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif

    Md3::initialize(app, opts);

    const QString appDir = QCoreApplication::applicationDirPath();
    QCoreApplication::addLibraryPath(appDir);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::warnings,
        &app,
        [](const QList<QQmlError> &warnings) {
            for (const QQmlError &e : warnings)
                qWarning("%s", qPrintable(e.toString()));
        });
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.addImportPath(appDir + QStringLiteral("/qml"));

#if defined(MD3_SHARED)
    const QString md3Plugin = appDir + QStringLiteral("/qml/Md3/Md3plugin.dll");
    if (QFile::exists(md3Plugin)) {
        QPluginLoader loader(md3Plugin);
        if (!loader.load()) {
            qCritical("Md3: failed to load %s: %s", qPrintable(md3Plugin),
                      qPrintable(loader.errorString()));
        }
    } else {
        qWarning("Md3: plugin not found at %s", qPrintable(md3Plugin));
    }
#endif

    const QString diskMain = appDir + QStringLiteral("/GitDesk/Main.qml");
    if (QFile::exists(diskMain))
        engine.load(QUrl::fromLocalFile(diskMain));
    else
        engine.loadFromModule(QStringLiteral("GitDesk"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty()) {
        qCritical("Failed to load Main.qml (import paths: %s)",
                  qPrintable(engine.importPathList().join(QLatin1String(", "))));
        return 1;
    }

    return app.exec();
}
