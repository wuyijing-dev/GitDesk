#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

struct GitResult {
    int exitCode = -1;
    QString stdoutText;
    QString stderrText;
    bool ok() const { return exitCode == 0; }
};

class GitRunner : public QObject
{
    Q_OBJECT
public:
    explicit GitRunner(QObject *parent = nullptr);

    QString gitExecutable() const;
    void setGitExecutable(const QString &path);

    GitResult run(const QString &workTree,
                  const QStringList &args,
                  int timeoutMs = 60000) const;

    QString findGit() const;

private:
    QString m_gitPath;
};
