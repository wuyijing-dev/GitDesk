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
    int conflictCount = 0;
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
    QVariantList remoteDetails() const { return m_remoteDetails; }
    QStringList tags() const { return m_tags; }
    QVariantList projectTree() const { return m_projectTree; }
    QVariantList recentActivity() const { return m_recentActivity; }
    QVariantList stashes() const { return m_stashes; }
    int ahead() const { return m_ahead; }
    int behind() const { return m_behind; }
    bool hasUpstream() const { return m_hasUpstream; }
    bool mergeInProgress() const { return m_mergeInProgress; }
    bool rebaseInProgress() const { return m_rebaseInProgress; }

    QString fileDiff(const QString &path, bool staged) const;
    QString commitDiff(const QString &commitId) const;
    CommitInfo commitById(const QString &id) const;
    QVariantList fileHistory(const QString &path, int limit = 30) const;
    QVariantList fileBlame(const QString &path) const;
    /// Compare tipA...tipB → { ahead, behind, commits: [...] }
    QVariantMap compareRefs(const QString &baseRef, const QString &headRef, int limit = 40) const;

    bool stage(const QString &path, QString *error = nullptr);
    bool unstage(const QString &path, QString *error = nullptr);
    bool stageAll(QString *error = nullptr);
    bool unstageAll(QString *error = nullptr);
    bool discard(const QString &path, QString *error = nullptr);
    bool discardAll(QString *error = nullptr);
    bool stashSave(const QString &message, QString *error = nullptr);
    bool stashPop(QString *error = nullptr);
    bool stashDrop(int index, QString *error = nullptr);
    bool commit(const QString &message, bool amend, QString *error = nullptr);
    bool checkout(const QString &ref, QString *error = nullptr);
    bool createBranch(const QString &name, QString *error = nullptr);
    bool createBranchAt(const QString &name, const QString &startPoint, QString *error = nullptr);
    bool deleteBranch(const QString &name, bool force, QString *error = nullptr);
    bool merge(const QString &branch, QString *error = nullptr);
    bool revertCommit(const QString &commitId, QString *error = nullptr);
    bool cherryPick(const QString &commitId, QString *error = nullptr);
    /// mode: "soft" | "mixed" | "hard"
    bool resetTo(const QString &commitId, const QString &mode, QString *error = nullptr);
    bool fetch(QString *error = nullptr);
    bool pull(QString *error = nullptr);
    bool pullRebase(QString *error = nullptr);
    bool push(QString *error = nullptr);
    bool pushSetUpstream(QString *error = nullptr);
    bool createTag(const QString &name, const QString &message = {}, QString *error = nullptr);
    bool deleteTag(const QString &name, QString *error = nullptr);
    /// Clone into destDir (full path of new repo folder), then open it.
    bool cloneRepo(const QString &url, const QString &destDir, QString *error = nullptr);
    /// git init in path, then open.
    bool initRepo(const QString &path, QString *error = nullptr);
    bool abortMerge(QString *error = nullptr);
    bool abortRebase(QString *error = nullptr);
    bool continueRebase(QString *error = nullptr);
    /// side: "ours" | "theirs" — checkout then stage
    bool resolveConflict(const QString &path, const QString &side, QString *error = nullptr);

signals:
    void changed();

private:
    bool loadIdentity(QString *error);
    bool loadBranches(QString *error);
    bool loadCommits(QString *error);
    bool loadStatus(QString *error);
    bool loadRemotesTags(QString *error);
    bool loadStats(QString *error);
    bool loadStashes(QString *error);
    bool loadAheadBehind(QString *error);
    void buildProjectTree();
    bool runOk(const QStringList &args, QString *error, QString *stdoutOut = nullptr,
               int timeoutMs = 60000) const;
    bool isUntracked(const QString &path) const;

    GitRunner *m_runner = nullptr;
    QString m_path;
    QString m_currentBranch;
    QString m_headShort;
    QList<CommitInfo> m_commits;
    QList<FileChange> m_changes;
    QList<BranchInfo> m_branches;
    QStringList m_remotes;
    QVariantList m_remoteDetails;
    QStringList m_tags;
    QVariantList m_projectTree;
    QVariantList m_recentActivity;
    QVariantList m_stashes;
    RepoStats m_stats;
    int m_ahead = 0;
    int m_behind = 0;
    bool m_hasUpstream = false;
    bool m_mergeInProgress = false;
    bool m_rebaseInProgress = false;
};
