#pragma once

#include "GitRunner.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QDateTime>

struct CommitInfo {
    QString id;
    QString shortId;
    QStringList parents;
    QString subject;
    QString body;
    QString authorName;
    QString authorEmail;
    QDateTime authorDate;
    QStringList refs;
};

struct FileChange {
    QString path;
    QString status; // M A D R C U ??
    bool staged = false;
    int additions = 0;
    int deletions = 0;
};

struct BranchInfo {
    QString name;
    bool current = false;
    bool remote = false;
    QString upstream;
    QString tip;
};

struct RepoStats {
    int commitCount = 0;
    int branchCount = 0;
    int tagCount = 0;
    int contributorCount = 0;
    int changedFileCount = 0;
};

class GitRepository : public QObject
{
    Q_OBJECT
public:
    explicit GitRepository(GitRunner *runner, QObject *parent = nullptr);

    QString path() const { return m_path; }
    bool isOpen() const { return !m_path.isEmpty(); }
    QString name() const;
    QString currentBranch() const { return m_currentBranch; }
    QString headShort() const { return m_headShort; }
    RepoStats stats() const { return m_stats; }

    bool open(const QString &path, QString *error = nullptr);
    void close();
    bool refresh(QString *error = nullptr);

    QList<CommitInfo> commits() const { return m_commits; }
    QList<FileChange> changes() const { return m_changes; }
    QList<BranchInfo> branches() const { return m_branches; }
    QStringList remotes() const { return m_remotes; }
    QStringList tags() const { return m_tags; }
    QVariantList projectTree() const { return m_projectTree; }
    QVariantList recentActivity() const { return m_recentActivity; }

    QString fileDiff(const QString &path, bool staged) const;
    QString commitDiff(const QString &commitId) const;
    CommitInfo commitById(const QString &id) const;

    bool stage(const QString &path, QString *error = nullptr);
    bool unstage(const QString &path, QString *error = nullptr);
    bool stageAll(QString *error = nullptr);
    bool unstageAll(QString *error = nullptr);
    bool commit(const QString &message, bool amend, QString *error = nullptr);
    bool checkout(const QString &ref, QString *error = nullptr);
    bool createBranch(const QString &name, QString *error = nullptr);
    bool deleteBranch(const QString &name, bool force, QString *error = nullptr);
    bool merge(const QString &branch, QString *error = nullptr);
    bool fetch(QString *error = nullptr);
    bool pull(QString *error = nullptr);
    bool push(QString *error = nullptr);

signals:
    void changed();

private:
    bool loadIdentity(QString *error);
    bool loadBranches(QString *error);
    bool loadCommits(QString *error);
    bool loadStatus(QString *error);
    bool loadRemotesTags(QString *error);
    bool loadStats(QString *error);
    void buildProjectTree();
    bool runOk(const QStringList &args, QString *error, QString *stdoutOut = nullptr) const;

    GitRunner *m_runner = nullptr;
    QString m_path;
    QString m_currentBranch;
    QString m_headShort;
    QList<CommitInfo> m_commits;
    QList<FileChange> m_changes;
    QList<BranchInfo> m_branches;
    QStringList m_remotes;
    QStringList m_tags;
    QVariantList m_projectTree;
    QVariantList m_recentActivity;
    RepoStats m_stats;
};
