#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>
#include <QMutex>
#include <QRecursiveMutex>
#include <functional>

#include "GitRunner.h"
#include "GitRepository.h"
#include "CommitListModel.h"
#include "FileChangeModel.h"
#include "BranchListModel.h"
#include "GraphNodeModel.h"
#include "VersionInfo.h"
#include "LocaleController.h"

class GitDeskApp : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(VersionInfo *versionInfo READ versionInfo CONSTANT)
    Q_PROPERTY(LocaleController *locale READ locale CONSTANT)

    Q_PROPERTY(bool hasRepo READ hasRepo NOTIFY repoChanged)
    Q_PROPERTY(QString repoPath READ repoPath NOTIFY repoChanged)
    Q_PROPERTY(QString repoName READ repoName NOTIFY repoChanged)
    Q_PROPERTY(QString currentBranch READ currentBranch NOTIFY repoChanged)
    Q_PROPERTY(QString headShort READ headShort NOTIFY repoChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString busyText READ busyText NOTIFY busyChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int workspaceTab READ workspaceTab WRITE setWorkspaceTab NOTIFY workspaceTabChanged)
    Q_PROPERTY(bool detailOpen READ detailOpen WRITE setDetailOpen NOTIFY detailOpenChanged)

    Q_PROPERTY(CommitListModel *commits READ commits CONSTANT)
    Q_PROPERTY(FileChangeModel *changes READ changes CONSTANT)
    Q_PROPERTY(BranchListModel *branches READ branches CONSTANT)
    Q_PROPERTY(GraphNodeModel *graph READ graph CONSTANT)

    Q_PROPERTY(int commitCount READ commitCount NOTIFY repoChanged)
    Q_PROPERTY(int branchCount READ branchCount NOTIFY repoChanged)
    Q_PROPERTY(int tagCount READ tagCount NOTIFY repoChanged)
    Q_PROPERTY(int contributorCount READ contributorCount NOTIFY repoChanged)
    Q_PROPERTY(int changedFileCount READ changedFileCount NOTIFY repoChanged)
    Q_PROPERTY(int conflictCount READ conflictCount NOTIFY repoChanged)
    Q_PROPERTY(QString currentDiff READ currentDiff NOTIFY selectionChanged)
    Q_PROPERTY(bool mergeInProgress READ mergeInProgress NOTIFY repoChanged)
    Q_PROPERTY(bool rebaseInProgress READ rebaseInProgress NOTIFY repoChanged)
    Q_PROPERTY(QVariantList remotes READ remotes NOTIFY repoChanged)
    Q_PROPERTY(QVariantList remoteDetails READ remoteDetails NOTIFY repoChanged)
    Q_PROPERTY(QVariantList tags READ tags NOTIFY repoChanged)
    Q_PROPERTY(QVariantList projectTree READ projectTree NOTIFY repoChanged)
    Q_PROPERTY(QVariantList recentActivity READ recentActivity NOTIFY repoChanged)
    Q_PROPERTY(QVariantList localBranchNames READ localBranchNames NOTIFY repoChanged)
    Q_PROPERTY(QVariantList stashes READ stashes NOTIFY repoChanged)
    Q_PROPERTY(int ahead READ ahead NOTIFY repoChanged)
    Q_PROPERTY(int behind READ behind NOTIFY repoChanged)
    Q_PROPERTY(bool hasUpstream READ hasUpstream NOTIFY repoChanged)

    Q_PROPERTY(QString selectedCommitId READ selectedCommitId WRITE setSelectedCommitId NOTIFY selectionChanged)
    Q_PROPERTY(QVariantMap selectedCommit READ selectedCommit NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedFilePath READ selectedFilePath WRITE setSelectedFilePath NOTIFY selectionChanged)
    Q_PROPERTY(bool selectedFileStaged READ selectedFileStaged WRITE setSelectedFileStaged NOTIFY selectionChanged)
    Q_PROPERTY(QVariantList fileHistory READ fileHistory NOTIFY selectionChanged)
    Q_PROPERTY(QVariantList fileBlame READ fileBlame NOTIFY selectionChanged)
    Q_PROPERTY(bool showBlame READ showBlame WRITE setShowBlame NOTIFY showBlameChanged)
    Q_PROPERTY(QVariantMap branchCompare READ branchCompare NOTIFY branchCompareChanged)
    Q_PROPERTY(bool diffLoading READ diffLoading NOTIFY diffLoadingChanged)
    Q_PROPERTY(QString commitMessage READ commitMessage WRITE setCommitMessage NOTIFY commitMessageChanged)
    Q_PROPERTY(bool amendCommit READ amendCommit WRITE setAmendCommit NOTIFY amendCommitChanged)
    Q_PROPERTY(QStringList recentRepos READ recentRepos NOTIFY recentReposChanged)

public:
    explicit GitDeskApp(QObject *parent = nullptr);
    static GitDeskApp *create(QQmlEngine *engine, QJSEngine *);
    static GitDeskApp *instance();

    bool hasRepo() const;
    QString repoPath() const;
    QString repoName() const;
    QString currentBranch() const;
    QString headShort() const;
    QString statusText() const { return m_statusText; }
    QString busyText() const { return m_busyText; }
    bool busy() const { return m_busy; }
    bool diffLoading() const { return m_diffLoading; }

    int workspaceTab() const { return m_workspaceTab; }
    void setWorkspaceTab(int tab);
    bool detailOpen() const { return m_detailOpen; }
    void setDetailOpen(bool open);

    VersionInfo *versionInfo() const { return m_versionInfo; }
    LocaleController *locale() const { return m_locale; }

    CommitListModel *commits() const { return m_commits; }
    FileChangeModel *changes() const { return m_changes; }
    BranchListModel *branches() const { return m_branches; }
    GraphNodeModel *graph() const { return m_graph; }

    int commitCount() const { return m_cachedStats.commitCount; }
    int branchCount() const { return m_cachedStats.branchCount; }
    int tagCount() const { return m_cachedStats.tagCount; }
    int contributorCount() const { return m_cachedStats.contributorCount; }
    int changedFileCount() const { return m_cachedStats.changedFileCount; }
    int conflictCount() const { return m_cachedStats.conflictCount; }
    bool mergeInProgress() const { return m_cachedMergeInProgress; }
    bool rebaseInProgress() const { return m_cachedRebaseInProgress; }

    QVariantList remotes() const { return m_cachedRemotes; }
    QVariantList remoteDetails() const { return m_cachedRemoteDetails; }
    QVariantList tags() const { return m_cachedTags; }
    QVariantList projectTree() const { return m_cachedProjectTree; }
    QVariantList recentActivity() const { return m_cachedRecentActivity; }
    QVariantList localBranchNames() const { return m_cachedLocalBranches; }
    QVariantList stashes() const { return m_cachedStashes; }
    int ahead() const { return m_cachedAhead; }
    int behind() const { return m_cachedBehind; }
    bool hasUpstream() const { return m_cachedHasUpstream; }

    QString selectedCommitId() const { return m_selectedCommitId; }
    void setSelectedCommitId(const QString &id);
    QVariantMap selectedCommit() const { return m_cachedSelectedCommit; }
    QString selectedFilePath() const { return m_selectedFilePath; }
    void setSelectedFilePath(const QString &path);
    bool selectedFileStaged() const { return m_selectedFileStaged; }
    void setSelectedFileStaged(bool staged);
    QString currentDiff() const { return m_currentDiff; }
    QVariantList fileHistory() const { return m_cachedFileHistory; }
    QVariantList fileBlame() const { return m_cachedFileBlame; }
    bool showBlame() const { return m_showBlame; }
    void setShowBlame(bool on);
    QVariantMap branchCompare() const { return m_cachedBranchCompare; }

    QString commitMessage() const { return m_commitMessage; }
    void setCommitMessage(const QString &msg);
    bool amendCommit() const { return m_amendCommit; }
    void setAmendCommit(bool amend);
    QStringList recentRepos() const { return m_recentRepos; }

    Q_INVOKABLE void openRepository(const QString &path);
    Q_INVOKABLE QString pickRepository();
    Q_INVOKABLE void closeRepository();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void fetch();
    Q_INVOKABLE void pull();
    Q_INVOKABLE void pullRebase();
    Q_INVOKABLE void push();
    Q_INVOKABLE void pushSetUpstream();
    Q_INVOKABLE void stageFile(const QString &path);
    Q_INVOKABLE void unstageFile(const QString &path);
    Q_INVOKABLE void stageAll();
    Q_INVOKABLE void unstageAll();
    Q_INVOKABLE void discardFile(const QString &path);
    Q_INVOKABLE void discardAll();
    Q_INVOKABLE void stashSave(const QString &message = {});
    Q_INVOKABLE void stashPop();
    Q_INVOKABLE void stashDrop(int index);
    Q_INVOKABLE void commit();
    Q_INVOKABLE void checkoutBranch(const QString &name);
    Q_INVOKABLE void createBranch(const QString &name);
    Q_INVOKABLE void createBranchAt(const QString &name, const QString &startPoint);
    Q_INVOKABLE void deleteBranch(const QString &name, bool force = false);
    Q_INVOKABLE void mergeBranch(const QString &name);
    Q_INVOKABLE void revertCommit(const QString &commitId);
    Q_INVOKABLE void cherryPickCommit(const QString &commitId);
    Q_INVOKABLE void resetToCommit(const QString &commitId, const QString &mode);
    Q_INVOKABLE void createTag(const QString &name, const QString &message = {});
    Q_INVOKABLE void deleteTag(const QString &name);
    Q_INVOKABLE void cloneRepository(const QString &url, const QString &destDir);
    Q_INVOKABLE QString pickCloneDirectory();
    Q_INVOKABLE void initRepository(const QString &path);
    Q_INVOKABLE QString pickAndInitRepository();
    Q_INVOKABLE void openRepoFolder();
    Q_INVOKABLE void openRemoteUrl(const QString &url);
    Q_INVOKABLE void abortMerge();
    Q_INVOKABLE void abortRebase();
    Q_INVOKABLE void continueRebase();
    Q_INVOKABLE void resolveConflict(const QString &path, const QString &side);
    Q_INVOKABLE void compareBranches(const QString &baseRef, const QString &headRef);
    Q_INVOKABLE void selectChange(const QString &path, bool staged);
    Q_INVOKABLE QString languageForPath(const QString &path) const;
    Q_INVOKABLE void clearRecentRepos();
    Q_INVOKABLE void setGitExecutable(const QString &path);
    Q_INVOKABLE QString gitExecutable() const;

signals:
    void repoChanged();
    void statusChanged();
    void busyChanged();
    void workspaceTabChanged();
    void detailOpenChanged();
    void selectionChanged();
    void showBlameChanged();
    void branchCompareChanged();
    void diffLoadingChanged();
    void commitMessageChanged();
    void amendCommitChanged();
    void recentReposChanged();
    void notify(const QString &message, const QString &severity);

private:
    using WorkerFn = std::function<QString()>; // empty = success, else error

    void setBusy(bool busy, const QString &text = {});
    void setStatus(const QString &text);
    void syncModels();
    void rememberRecent(const QString &path);
    void loadRecents();
    void saveRecents();
    void updateDiffAsync();
    void refreshSelectedCommitCache();

    /// Run git work off the UI thread. UI keeps showing cached models until done.
    void runAsync(const QString &busyText,
                  WorkerFn worker,
                  const QString &successMessage = {},
                  std::function<void()> onSuccess = nullptr);

    static GitDeskApp *s_instance;

    GitRunner m_runner;
    GitRepository m_repo;
    QRecursiveMutex m_repoMutex;
    VersionInfo *m_versionInfo = nullptr;
    LocaleController *m_locale = nullptr;
    CommitListModel *m_commits = nullptr;
    FileChangeModel *m_changes = nullptr;
    BranchListModel *m_branches = nullptr;
    GraphNodeModel *m_graph = nullptr;

    // UI-facing cache (main thread only after syncModels)
    bool m_hasRepo = false;
    QString m_cachedPath;
    QString m_cachedName;
    QString m_cachedBranch;
    QString m_cachedHeadShort;
    RepoStats m_cachedStats;
    QVariantList m_cachedRemotes;
    QVariantList m_cachedRemoteDetails;
    QVariantList m_cachedTags;
    QVariantList m_cachedProjectTree;
    QVariantList m_cachedRecentActivity;
    QVariantList m_cachedLocalBranches;
    QVariantList m_cachedStashes;
    QVariantMap m_cachedSelectedCommit;
    int m_cachedAhead = 0;
    int m_cachedBehind = 0;
    bool m_cachedHasUpstream = false;
    bool m_cachedMergeInProgress = false;
    bool m_cachedRebaseInProgress = false;
    QVariantList m_cachedFileHistory;
    QVariantList m_cachedFileBlame;
    bool m_showBlame = false;
    QVariantMap m_cachedBranchCompare;
    QVariantMap m_pendingBranchCompare;

    QString m_statusText;
    QString m_busyText;
    bool m_busy = false;
    int m_asyncToken = 0;
    int m_workspaceTab = 0;
    bool m_detailOpen = true;
    QString m_selectedCommitId;
    QString m_selectedFilePath;
    bool m_selectedFileStaged = false;
    QString m_currentDiff;
    bool m_diffLoading = false;
    int m_diffToken = 0;
    QString m_commitMessage;
    bool m_amendCommit = false;
    QStringList m_recentRepos;
};
