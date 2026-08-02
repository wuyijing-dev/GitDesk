#include "GitDeskApp.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QSettings>
#include <QFileInfo>
#include <QtConcurrent>
#include <QMetaObject>
#include <QMutexLocker>

GitDeskApp *GitDeskApp::s_instance = nullptr;

GitDeskApp::GitDeskApp(QObject *parent)
    : QObject(parent)
    , m_repo(&m_runner)
{
    s_instance = this;
    m_versionInfo = new VersionInfo(this);
    m_locale = new LocaleController(this);
    m_commits = new CommitListModel(this);
    m_changes = new FileChangeModel(this);
    m_branches = new BranchListModel(this);
    m_graph = new GraphNodeModel(this);
    loadRecents();
    setStatus(tr("Open a repository to begin"));
}

GitDeskApp *GitDeskApp::create(QQmlEngine *engine, QJSEngine *)
{
    if (!s_instance)
        s_instance = new GitDeskApp(QCoreApplication::instance());
    if (engine && s_instance->m_locale)
        s_instance->m_locale->setEngine(engine);
    QQmlEngine::setObjectOwnership(s_instance, QQmlEngine::CppOwnership);
    return s_instance;
}

GitDeskApp *GitDeskApp::instance()
{
    return s_instance;
}

bool GitDeskApp::hasRepo() const
{
    return m_hasRepo;
}

QString GitDeskApp::repoPath() const
{
    return m_cachedPath;
}

QString GitDeskApp::repoName() const
{
    return m_cachedName;
}

QString GitDeskApp::currentBranch() const
{
    return m_cachedBranch;
}

QString GitDeskApp::headShort() const
{
    return m_cachedHeadShort;
}

void GitDeskApp::setWorkspaceTab(int tab)
{
    if (m_workspaceTab == tab)
        return;
    m_workspaceTab = tab;
    emit workspaceTabChanged();
}

void GitDeskApp::setDetailOpen(bool open)
{
    if (m_detailOpen == open)
        return;
    m_detailOpen = open;
    emit detailOpenChanged();
}

void GitDeskApp::setSelectedCommitId(const QString &id)
{
    if (m_selectedCommitId == id)
        return;
    m_selectedCommitId = id;
    m_selectedFilePath.clear();
    refreshSelectedCommitCache();
    if (!id.isEmpty()) {
        setDetailOpen(true);
        updateDiffAsync();
    } else {
        m_currentDiff.clear();
        emit selectionChanged();
    }
}

void GitDeskApp::setSelectedFilePath(const QString &path)
{
    if (m_selectedFilePath == path)
        return;
    m_selectedFilePath = path;
    updateDiffAsync();
    emit selectionChanged();
}

void GitDeskApp::setSelectedFileStaged(bool staged)
{
    if (m_selectedFileStaged == staged)
        return;
    m_selectedFileStaged = staged;
    updateDiffAsync();
    emit selectionChanged();
}

void GitDeskApp::setCommitMessage(const QString &msg)
{
    if (m_commitMessage == msg)
        return;
    m_commitMessage = msg;
    emit commitMessageChanged();
}

void GitDeskApp::setAmendCommit(bool amend)
{
    if (m_amendCommit == amend)
        return;
    m_amendCommit = amend;
    emit amendCommitChanged();
}

void GitDeskApp::setBusy(bool busy, const QString &text)
{
    m_busy = busy;
    m_busyText = text;
    emit busyChanged();
}

void GitDeskApp::setStatus(const QString &text)
{
    if (m_statusText == text)
        return;
    m_statusText = text;
    emit statusChanged();
}

void GitDeskApp::refreshSelectedCommitCache()
{
    QMutexLocker lock(&m_repoMutex);
    m_cachedSelectedCommit.clear();
    if (m_selectedCommitId.isEmpty())
        return;
    const CommitInfo c = m_repo.commitById(m_selectedCommitId);
    if (c.id.isEmpty())
        return;
    m_cachedSelectedCommit.insert(QStringLiteral("id"), c.id);
    m_cachedSelectedCommit.insert(QStringLiteral("shortId"), c.shortId);
    m_cachedSelectedCommit.insert(QStringLiteral("subject"), c.subject);
    m_cachedSelectedCommit.insert(QStringLiteral("body"), c.body);
    m_cachedSelectedCommit.insert(QStringLiteral("author"), c.authorName);
    m_cachedSelectedCommit.insert(QStringLiteral("email"), c.authorEmail);
    m_cachedSelectedCommit.insert(QStringLiteral("date"),
                                  c.authorDate.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
    m_cachedSelectedCommit.insert(QStringLiteral("refs"), QVariant::fromValue(c.refs));
}

void GitDeskApp::syncModels()
{
    QMutexLocker lock(&m_repoMutex);
    m_hasRepo = m_repo.isOpen();
    m_cachedPath = m_repo.path();
    m_cachedName = m_repo.name();
    m_cachedBranch = m_repo.currentBranch();
    m_cachedHeadShort = m_repo.headShort();
    m_cachedStats = m_repo.stats();

    m_cachedRemotes.clear();
    for (const QString &r : m_repo.remotes())
        m_cachedRemotes.push_back(r);

    m_cachedTags.clear();
    for (const QString &t : m_repo.tags())
        m_cachedTags.push_back(t);

    m_cachedProjectTree = m_repo.projectTree();
    m_cachedRecentActivity = m_repo.recentActivity();
    m_cachedStashes = m_repo.stashes();
    m_cachedAhead = m_repo.ahead();
    m_cachedBehind = m_repo.behind();
    m_cachedHasUpstream = m_repo.hasUpstream();

    m_cachedLocalBranches.clear();
    for (const BranchInfo &b : m_repo.branches()) {
        if (!b.remote)
            m_cachedLocalBranches.push_back(b.name);
    }

    m_commits->setCommits(m_repo.commits());
    m_changes->setChanges(m_repo.changes());
    m_branches->setBranches(m_repo.branches());
    m_graph->rebuild(m_repo.commits());

    m_cachedSelectedCommit.clear();
    if (!m_selectedCommitId.isEmpty()) {
        const CommitInfo c = m_repo.commitById(m_selectedCommitId);
        if (!c.id.isEmpty()) {
            m_cachedSelectedCommit.insert(QStringLiteral("id"), c.id);
            m_cachedSelectedCommit.insert(QStringLiteral("shortId"), c.shortId);
            m_cachedSelectedCommit.insert(QStringLiteral("subject"), c.subject);
            m_cachedSelectedCommit.insert(QStringLiteral("body"), c.body);
            m_cachedSelectedCommit.insert(QStringLiteral("author"), c.authorName);
            m_cachedSelectedCommit.insert(QStringLiteral("email"), c.authorEmail);
            m_cachedSelectedCommit.insert(QStringLiteral("date"),
                                          c.authorDate.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
            m_cachedSelectedCommit.insert(QStringLiteral("refs"), QVariant::fromValue(c.refs));
        }
    }

    emit repoChanged();
    emit selectionChanged();
}

void GitDeskApp::runAsync(const QString &busyText,
                          WorkerFn worker,
                          const QString &successMessage,
                          std::function<void()> onSuccess)
{
    if (m_busy) {
        emit notify(tr("请等待当前操作完成"), QStringLiteral("error"));
        return;
    }

    setBusy(true, busyText);
    const int token = ++m_asyncToken;

    (void)QtConcurrent::run([this, worker, successMessage, onSuccess, token]() {
        QString error;
        {
            QMutexLocker lock(&m_repoMutex);
            const bool blocked = m_repo.blockSignals(true);
            error = worker ? worker() : QStringLiteral("internal error");
            m_repo.blockSignals(blocked);
        }

        QMetaObject::invokeMethod(this, [this, error, successMessage, onSuccess, token]() {
            if (token != m_asyncToken)
                return;

            if (!error.isEmpty()) {
                setBusy(false);
                emit notify(error, QStringLiteral("error"));
                return;
            }

            syncModels();
            setBusy(false);
            if (m_hasRepo) {
                setStatus(tr("%1 · %2 · %3 changes")
                              .arg(m_cachedName, m_cachedBranch)
                              .arg(m_cachedStats.changedFileCount));
            }
            if (onSuccess)
                onSuccess();
            if (!successMessage.isEmpty())
                emit notify(successMessage, QStringLiteral("success"));
        }, Qt::QueuedConnection);
    });
}

void GitDeskApp::updateDiffAsync()
{
    const QString filePath = m_selectedFilePath;
    const bool staged = m_selectedFileStaged;
    const QString commitId = m_selectedCommitId;

    if ((!m_hasRepo && !m_repo.isOpen()) || (filePath.isEmpty() && commitId.isEmpty())) {
        m_currentDiff.clear();
        if (m_diffLoading) {
            m_diffLoading = false;
            emit diffLoadingChanged();
        }
        emit selectionChanged();
        return;
    }

    const int token = ++m_diffToken;
    if (!m_diffLoading) {
        m_diffLoading = true;
        emit diffLoadingChanged();
    }

    (void)QtConcurrent::run([this, token, filePath, staged, commitId]() {
        QString diff;
        {
            QMutexLocker lock(&m_repoMutex);
            if (!filePath.isEmpty())
                diff = m_repo.fileDiff(filePath, staged);
            else if (!commitId.isEmpty())
                diff = m_repo.commitDiff(commitId);
        }

        QMetaObject::invokeMethod(this, [this, token, diff]() {
            if (token != m_diffToken)
                return;
            m_currentDiff = diff;
            m_diffLoading = false;
            emit diffLoadingChanged();
            emit selectionChanged();
        }, Qt::QueuedConnection);
    });
}

void GitDeskApp::loadRecents()
{
    QSettings s;
    m_recentRepos = s.value(QStringLiteral("recentRepos")).toStringList();
    emit recentReposChanged();
}

void GitDeskApp::saveRecents()
{
    QSettings s;
    s.setValue(QStringLiteral("recentRepos"), m_recentRepos);
}

void GitDeskApp::rememberRecent(const QString &path)
{
    m_recentRepos.removeAll(path);
    m_recentRepos.prepend(path);
    while (m_recentRepos.size() > 12)
        m_recentRepos.removeLast();
    saveRecents();
    emit recentReposChanged();
}

void GitDeskApp::clearRecentRepos()
{
    m_recentRepos.clear();
    saveRecents();
    emit recentReposChanged();
}

void GitDeskApp::setGitExecutable(const QString &path)
{
    if (path.trimmed().isEmpty())
        m_runner.setGitExecutable(m_runner.findGit());
    else
        m_runner.setGitExecutable(path.trimmed());
}

QString GitDeskApp::gitExecutable() const
{
    return m_runner.gitExecutable();
}

void GitDeskApp::openRepository(const QString &path)
{
    if (path.isEmpty())
        return;

    const QString abs = QFileInfo(path).absoluteFilePath();
    runAsync(tr("正在打开仓库…"), [this, abs]() {
        QString error;
        if (!m_repo.open(abs, &error))
            return error.isEmpty() ? tr("无法打开仓库") : error;
        return QString();
    }, tr("已打开仓库"), [this, abs]() {
        rememberRecent(m_cachedPath.isEmpty() ? abs : m_cachedPath);
        m_selectedCommitId.clear();
        m_selectedFilePath.clear();
        m_commitMessage.clear();
        m_currentDiff.clear();
        emit commitMessageChanged();
        emit selectionChanged();
    });
}

QString GitDeskApp::pickRepository()
{
    const QString dir = QFileDialog::getExistingDirectory(
        nullptr, tr("Open Git Repository"),
        m_recentRepos.isEmpty() ? QString() : m_recentRepos.first());
    if (dir.isEmpty())
        return {};
    openRepository(dir);
    return dir;
}

void GitDeskApp::closeRepository()
{
    if (m_busy) {
        emit notify(tr("请等待当前操作完成"), QStringLiteral("error"));
        return;
    }
    ++m_asyncToken;
    ++m_diffToken;
    {
        QMutexLocker lock(&m_repoMutex);
        m_repo.close();
    }
    m_hasRepo = false;
    m_cachedPath.clear();
    m_cachedName.clear();
    m_cachedBranch.clear();
    m_cachedHeadShort.clear();
    m_cachedStats = {};
    m_cachedRemotes.clear();
    m_cachedTags.clear();
    m_cachedProjectTree.clear();
    m_cachedRecentActivity.clear();
    m_cachedLocalBranches.clear();
    m_cachedStashes.clear();
    m_cachedSelectedCommit.clear();
    m_cachedAhead = 0;
    m_cachedBehind = 0;
    m_cachedHasUpstream = false;
    m_commits->clear();
    m_changes->clear();
    m_branches->clear();
    m_graph->clear();
    m_selectedCommitId.clear();
    m_selectedFilePath.clear();
    m_currentDiff.clear();
    m_diffLoading = false;
    setStatus(tr("No repository open"));
    emit diffLoadingChanged();
    emit repoChanged();
    emit selectionChanged();
}

void GitDeskApp::refresh()
{
    if (!m_hasRepo && !m_repo.isOpen())
        return;
    runAsync(tr("正在刷新…"), [this]() {
        QString error;
        if (!m_repo.refresh(&error))
            return error.isEmpty() ? tr("刷新失败") : error;
        return QString();
    });
}

void GitDeskApp::fetch()
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("正在 Fetch…"), [this]() {
        QString error;
        if (!m_repo.fetch(&error))
            return error.isEmpty() ? tr("Fetch 失败") : error;
        return QString();
    }, tr("Fetch 完成"));
}

void GitDeskApp::pull()
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("正在 Pull…"), [this]() {
        QString error;
        if (!m_repo.pull(&error))
            return error.isEmpty() ? tr("Pull 失败") : error;
        return QString();
    }, tr("Pull 完成"));
}

void GitDeskApp::push()
{
    if (!m_repo.isOpen())
        return;
    if (!m_cachedHasUpstream) {
        pushSetUpstream();
        return;
    }
    runAsync(tr("正在 Push…"), [this]() {
        QString error;
        if (!m_repo.push(&error))
            return error.isEmpty() ? tr("Push 失败") : error;
        return QString();
    }, tr("Push 完成"));
}

void GitDeskApp::pushSetUpstream()
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("正在 Push 并设置上游…"), [this]() {
        QString error;
        if (!m_repo.pushSetUpstream(&error))
            return error.isEmpty() ? tr("Push 失败") : error;
        return QString();
    }, tr("已推送并设置上游"));
}

void GitDeskApp::stageFile(const QString &path)
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("Staging…"), [this, path]() {
        QString error;
        if (!m_repo.stage(path, &error))
            return error.isEmpty() ? tr("Stage 失败") : error;
        return QString();
    }, tr("已暂存"));
}

void GitDeskApp::unstageFile(const QString &path)
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("Unstaging…"), [this, path]() {
        QString error;
        if (!m_repo.unstage(path, &error))
            return error.isEmpty() ? tr("Unstage 失败") : error;
        return QString();
    }, tr("已取消暂存"));
}

void GitDeskApp::stageAll()
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("Staging all…"), [this]() {
        QString error;
        if (!m_repo.stageAll(&error))
            return error.isEmpty() ? tr("Stage all 失败") : error;
        return QString();
    }, tr("已全部暂存"));
}

void GitDeskApp::unstageAll()
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("Unstaging all…"), [this]() {
        QString error;
        if (!m_repo.unstageAll(&error))
            return error.isEmpty() ? tr("Unstage all 失败") : error;
        return QString();
    }, tr("已全部取消暂存"));
}

void GitDeskApp::discardFile(const QString &path)
{
    if (!m_repo.isOpen() || path.isEmpty())
        return;
    runAsync(tr("丢弃变更…"), [this, path]() {
        QString error;
        if (!m_repo.discard(path, &error))
            return error.isEmpty() ? tr("丢弃失败") : error;
        return QString();
    }, tr("已丢弃 %1").arg(path), [this, path]() {
        if (m_selectedFilePath == path) {
            m_selectedFilePath.clear();
            m_currentDiff.clear();
            emit selectionChanged();
        }
    });
}

void GitDeskApp::discardAll()
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("丢弃全部未暂存…"), [this]() {
        QString error;
        if (!m_repo.discardAll(&error))
            return error.isEmpty() ? tr("丢弃失败") : error;
        return QString();
    }, tr("已丢弃未暂存变更"));
}

void GitDeskApp::stashSave(const QString &message)
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("Stash…"), [this, message]() {
        QString error;
        if (!m_repo.stashSave(message, &error))
            return error.isEmpty() ? tr("Stash 失败") : error;
        return QString();
    }, tr("已保存 Stash"));
}

void GitDeskApp::stashPop()
{
    if (!m_repo.isOpen())
        return;
    runAsync(tr("Stash Pop…"), [this]() {
        QString error;
        if (!m_repo.stashPop(&error))
            return error.isEmpty() ? tr("Stash Pop 失败") : error;
        return QString();
    }, tr("已弹出 Stash"));
}

void GitDeskApp::stashDrop(int index)
{
    if (!m_repo.isOpen() || index < 0)
        return;
    runAsync(tr("删除 Stash…"), [this, index]() {
        QString error;
        if (!m_repo.stashDrop(index, &error))
            return error.isEmpty() ? tr("删除 Stash 失败") : error;
        return QString();
    }, tr("已删除 Stash"));
}

void GitDeskApp::commit()
{
    if (!m_repo.isOpen())
        return;
    const QString message = m_commitMessage;
    const bool amend = m_amendCommit;
    runAsync(tr("正在提交…"), [this, message, amend]() {
        QString error;
        if (!m_repo.commit(message, amend, &error))
            return error.isEmpty() ? tr("提交失败") : error;
        return QString();
    }, tr("提交成功"), [this]() {
        m_commitMessage.clear();
        m_amendCommit = false;
        emit commitMessageChanged();
        emit amendCommitChanged();
        setWorkspaceTab(1);
    });
}

void GitDeskApp::checkoutBranch(const QString &name)
{
    if (!m_repo.isOpen() || name.isEmpty())
        return;
    runAsync(tr("切换分支…"), [this, name]() {
        QString error;
        if (!m_repo.checkout(name, &error))
            return error.isEmpty() ? tr("切换失败") : error;
        return QString();
    }, tr("已切换到 %1").arg(name));
}

void GitDeskApp::createBranch(const QString &name)
{
    if (!m_repo.isOpen() || name.isEmpty())
        return;
    runAsync(tr("创建分支…"), [this, name]() {
        QString error;
        if (!m_repo.createBranch(name, &error))
            return error.isEmpty() ? tr("创建失败") : error;
        return QString();
    }, tr("已创建 %1").arg(name));
}

void GitDeskApp::deleteBranch(const QString &name, bool force)
{
    if (!m_repo.isOpen() || name.isEmpty())
        return;
    runAsync(tr("删除分支…"), [this, name, force]() {
        QString error;
        if (!m_repo.deleteBranch(name, force, &error))
            return error.isEmpty() ? tr("删除失败") : error;
        return QString();
    }, tr("已删除 %1").arg(name));
}

void GitDeskApp::mergeBranch(const QString &name)
{
    if (!m_repo.isOpen() || name.isEmpty())
        return;
    runAsync(tr("Merge…"), [this, name]() {
        QString error;
        if (!m_repo.merge(name, &error))
            return error.isEmpty() ? tr("Merge 失败") : error;
        return QString();
    }, tr("已合并 %1").arg(name));
}

void GitDeskApp::createTag(const QString &name, const QString &message)
{
    if (!m_repo.isOpen() || name.trimmed().isEmpty())
        return;
    runAsync(tr("创建标签…"), [this, name, message]() {
        QString error;
        if (!m_repo.createTag(name, message, &error))
            return error.isEmpty() ? tr("创建标签失败") : error;
        return QString();
    }, tr("已创建标签 %1").arg(name.trimmed()));
}

void GitDeskApp::deleteTag(const QString &name)
{
    if (!m_repo.isOpen() || name.trimmed().isEmpty())
        return;
    runAsync(tr("删除标签…"), [this, name]() {
        QString error;
        if (!m_repo.deleteTag(name, &error))
            return error.isEmpty() ? tr("删除标签失败") : error;
        return QString();
    }, tr("已删除标签 %1").arg(name.trimmed()));
}

void GitDeskApp::cloneRepository(const QString &url, const QString &destDir)
{
    if (url.trimmed().isEmpty() || destDir.trimmed().isEmpty()) {
        emit notify(tr("请填写克隆地址与目标目录"), QStringLiteral("error"));
        return;
    }
    const QString u = url.trimmed();
    const QString d = QFileInfo(destDir.trimmed()).absoluteFilePath();
    runAsync(tr("正在克隆…"), [this, u, d]() {
        QString error;
        if (!m_repo.cloneRepo(u, d, &error))
            return error.isEmpty() ? tr("克隆失败") : error;
        return QString();
    }, tr("克隆完成"), [this, d]() {
        rememberRecent(m_cachedPath.isEmpty() ? d : m_cachedPath);
        m_selectedCommitId.clear();
        m_selectedFilePath.clear();
        m_commitMessage.clear();
        m_currentDiff.clear();
        emit commitMessageChanged();
        emit selectionChanged();
    });
}

QString GitDeskApp::pickCloneDirectory()
{
    const QString dir = QFileDialog::getExistingDirectory(
        nullptr, tr("选择克隆目标父目录"),
        m_recentRepos.isEmpty() ? QString() : m_recentRepos.first());
    return dir;
}

void GitDeskApp::initRepository(const QString &path)
{
    if (path.trimmed().isEmpty())
        return;
    const QString abs = QFileInfo(path).absoluteFilePath();
    runAsync(tr("正在初始化仓库…"), [this, abs]() {
        QString error;
        if (!m_repo.initRepo(abs, &error))
            return error.isEmpty() ? tr("初始化失败") : error;
        return QString();
    }, tr("仓库已初始化"), [this, abs]() {
        rememberRecent(m_cachedPath.isEmpty() ? abs : m_cachedPath);
        m_selectedCommitId.clear();
        m_selectedFilePath.clear();
        m_commitMessage.clear();
        m_currentDiff.clear();
        emit commitMessageChanged();
        emit selectionChanged();
    });
}

QString GitDeskApp::pickAndInitRepository()
{
    const QString dir = QFileDialog::getExistingDirectory(
        nullptr, tr("选择要初始化的文件夹"),
        m_recentRepos.isEmpty() ? QString() : m_recentRepos.first());
    if (dir.isEmpty())
        return {};
    initRepository(dir);
    return dir;
}

void GitDeskApp::selectChange(const QString &path, bool staged)
{
    m_selectedCommitId.clear();
    m_selectedFilePath = path;
    m_selectedFileStaged = staged;
    m_cachedSelectedCommit.clear();
    setDetailOpen(true);
    emit selectionChanged();
    updateDiffAsync();
}

QString GitDeskApp::languageForPath(const QString &path) const
{
    const QString ext = QFileInfo(path).suffix().toLower();
    if (ext == QLatin1String("qml") || ext == QLatin1String("js"))
        return QStringLiteral("qml");
    if (ext == QLatin1String("cpp") || ext == QLatin1String("cc") || ext == QLatin1String("cxx")
        || ext == QLatin1String("h") || ext == QLatin1String("hpp") || ext == QLatin1String("c"))
        return QStringLiteral("cpp");
    if (ext == QLatin1String("py"))
        return QStringLiteral("plain");
    if (ext == QLatin1String("json"))
        return QStringLiteral("json");
    if (ext == QLatin1String("md") || ext == QLatin1String("markdown"))
        return QStringLiteral("plain");
    return QStringLiteral("plain");
}
