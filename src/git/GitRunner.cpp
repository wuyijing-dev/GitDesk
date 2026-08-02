#include "GitRunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

GitRunner::GitRunner(QObject *parent)
    : QObject(parent)
{
    m_gitPath = findGit();
}

QString GitRunner::gitExecutable() const
{
    return m_gitPath;
}

void GitRunner::setGitExecutable(const QString &path)
{
    m_gitPath = path;
}

QString GitRunner::findGit() const
{
    const QString fromPath = QStandardPaths::findExecutable(QStringLiteral("git"));
    if (!fromPath.isEmpty())
        return fromPath;

#ifdef Q_OS_WIN
    const QStringList candidates = {
        QStringLiteral("C:/Program Files/Git/cmd/git.exe"),
        QStringLiteral("C:/Program Files (x86)/Git/cmd/git.exe"),
        QStringLiteral("C:/Program Files/Git/bin/git.exe"),
    };
    for (const QString &c : candidates) {
        if (QFileInfo::exists(c))
            return QDir::toNativeSeparators(c);
    }
#endif
    return QStringLiteral("git");
}

GitResult GitRunner::run(const QString &workTree,
                         const QStringList &args,
                         int timeoutMs) const
{
    GitResult result;
    if (m_gitPath.isEmpty()) {
        result.stderrText = QStringLiteral("git executable not found");
        return result;
    }

    QStringList fullArgs;
    if (!workTree.isEmpty()) {
        fullArgs << QStringLiteral("-C") << workTree;
    }
    fullArgs << args;

    QProcess proc;
    proc.setProgram(m_gitPath);
    proc.setArguments(fullArgs);
    proc.setProcessChannelMode(QProcess::SeparateChannels);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("GIT_TERMINAL_PROMPT"), QStringLiteral("0"));
    env.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
    proc.setProcessEnvironment(env);

    proc.start();
    if (!proc.waitForStarted(5000)) {
        result.stderrText = proc.errorString();
        return result;
    }
    if (!proc.waitForFinished(timeoutMs)) {
        proc.kill();
        proc.waitForFinished(2000);
        result.stderrText = QStringLiteral("git timed out");
        return result;
    }

    result.exitCode = proc.exitCode();
    result.stdoutText = QString::fromUtf8(proc.readAllStandardOutput());
    result.stderrText = QString::fromUtf8(proc.readAllStandardError());
    return result;
}
