#include "GitRepository.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>

GitRepository::GitRepository(GitRunner *runner, QObject *parent)
    : QObject(parent)
    , m_runner(runner)
{
}

QString GitRepository::name() const
{
    if (m_path.isEmpty())
        return {};
    return QFileInfo(m_path).fileName();
}

bool GitRepository::runOk(const QStringList &args, QString *error, QString *stdoutOut, int timeoutMs) const
{
    const GitResult r = m_runner->run(m_path, args, timeoutMs);
    if (stdoutOut)
        *stdoutOut = r.stdoutText;
    if (!r.ok()) {
        if (error) {
            *error = r.stderrText.trimmed();
            if (error->isEmpty())
                *error = QStringLiteral("git failed (%1)").arg(r.exitCode);
        }
        return false;
    }
    return true;
}

bool GitRepository::open(const QString &path, QString *error)
{
    const QString cleaned = QDir::cleanPath(path);
    if (!QFileInfo::exists(cleaned)) {
        if (error)
            *error = QStringLiteral("Path does not exist");
        return false;
    }

    const GitResult r = m_runner->run(cleaned, {QStringLiteral("rev-parse"), QStringLiteral("--is-inside-work-tree")});
    if (!r.ok() || !r.stdoutText.trimmed().startsWith(QLatin1String("true"))) {
        if (error)
            *error = QStringLiteral("Not a git repository");
        return false;
    }

    const GitResult top = m_runner->run(cleaned, {QStringLiteral("rev-parse"), QStringLiteral("--show-toplevel")});
    m_path = QDir::fromNativeSeparators(top.ok() ? top.stdoutText.trimmed() : cleaned);
    return refresh(error);
}

void GitRepository::close()
{
    m_path.clear();
    m_currentBranch.clear();
    m_headShort.clear();
    m_commits.clear();
    m_changes.clear();
    m_branches.clear();
    m_remotes.clear();
    m_remoteDetails.clear();
    m_tags.clear();
    m_projectTree.clear();
    m_recentActivity.clear();
    m_stashes.clear();
    m_stats = {};
    m_ahead = 0;
    m_behind = 0;
    m_hasUpstream = false;
    m_mergeInProgress = false;
    m_rebaseInProgress = false;
    emit changed();
}

bool GitRepository::refresh(QString *error)
{
    if (m_path.isEmpty())
        return false;
    if (!loadIdentity(error))
        return false;
    if (!loadBranches(error))
        return false;
    if (!loadCommits(error))
        return false;
    if (!loadStatus(error))
        return false;
    if (!loadRemotesTags(error))
        return false;
    loadStashes(error);
    loadAheadBehind(error);
    const QDir gitDir(QDir(m_path).filePath(QStringLiteral(".git")));
    m_mergeInProgress = QFileInfo::exists(gitDir.filePath(QStringLiteral("MERGE_HEAD")));
    m_rebaseInProgress = QFileInfo::exists(gitDir.filePath(QStringLiteral("rebase-merge")))
                         || QFileInfo::exists(gitDir.filePath(QStringLiteral("rebase-apply")));
    loadStats(error);
    buildProjectTree();
    emit changed();
    return true;
}

bool GitRepository::loadIdentity(QString *error)
{
    QString out;
    if (!runOk({QStringLiteral("rev-parse"), QStringLiteral("--abbrev-ref"), QStringLiteral("HEAD")}, error, &out))
        return false;
    m_currentBranch = out.trimmed();
    if (m_currentBranch == QLatin1String("HEAD"))
        m_currentBranch = QStringLiteral("(detached)");

    if (!runOk({QStringLiteral("rev-parse"), QStringLiteral("--short"), QStringLiteral("HEAD")}, error, &out)) {
        m_headShort.clear();
    } else {
        m_headShort = out.trimmed();
    }
    return true;
}

bool GitRepository::loadBranches(QString *error)
{
    QString out;
    if (!runOk({QStringLiteral("for-each-ref"),
                QStringLiteral("--format=%(refname:short)\t%(objectname:short)\t%(HEAD)\t%(upstream:short)\t%(refname)"),
                QStringLiteral("refs/heads"), QStringLiteral("refs/remotes")},
               error, &out)) {
        return false;
    }

    m_branches.clear();
    const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QStringList parts = line.split(QLatin1Char('\t'));
        if (parts.size() < 5)
            continue;
        BranchInfo b;
        b.name = parts[0];
        b.tip = parts[1];
        b.current = parts[2] == QLatin1String("*");
        b.upstream = parts[3];
        b.remote = parts[4].startsWith(QLatin1String("refs/remotes/"));
        if (b.remote && (b.name.endsWith(QLatin1String("/HEAD")) || b.name == QLatin1String("origin/HEAD")))
            continue;
        m_branches.push_back(b);
    }
    return true;
}

bool GitRepository::loadCommits(QString *error)
{
    // hash|parents|subject|body|author|email|unix|refs
    const QString fmt = QStringLiteral("%H%x1f%P%x1f%s%x1f%b%x1f%an%x1f%ae%x1f%at%x1f%D%x1e");
    QString out;
    if (!runOk({QStringLiteral("log"), QStringLiteral("--all"), QStringLiteral("--topo-order"),
                QStringLiteral("--date=unix"), QStringLiteral("--pretty=format:") + fmt,
                QStringLiteral("-n"), QStringLiteral("200")},
               error, &out)) {
        return false;
    }

    m_commits.clear();
    m_recentActivity.clear();
    const QStringList records = out.split(QChar(0x1e), Qt::SkipEmptyParts);
    for (QString rec : records) {
        rec = rec.trimmed();
        if (rec.isEmpty())
            continue;
        const QStringList f = rec.split(QChar(0x1f));
        if (f.size() < 7)
            continue;
        CommitInfo c;
        c.id = f[0].trimmed();
        c.shortId = c.id.left(7);
        const QString parents = f[1].trimmed();
        if (!parents.isEmpty())
            c.parents = parents.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        c.subject = f[2];
        c.body = f.value(3);
        c.authorName = f.value(4);
        c.authorEmail = f.value(5);
        c.authorDate = QDateTime::fromSecsSinceEpoch(f.value(6).toLongLong());
        const QString refs = f.value(7).trimmed();
        if (!refs.isEmpty()) {
            for (QString r : refs.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
                r = r.trimmed();
                if (r.startsWith(QLatin1String("HEAD -> ")))
                    r = r.mid(8);
                if (!r.isEmpty())
                    c.refs.push_back(r);
            }
        }
        m_commits.push_back(c);

        if (m_recentActivity.size() < 8) {
            QVariantMap act;
            act.insert(QStringLiteral("id"), c.shortId);
            act.insert(QStringLiteral("title"), c.subject);
            act.insert(QStringLiteral("author"), c.authorName);
            act.insert(QStringLiteral("date"), c.authorDate.toString(Qt::ISODate));
            m_recentActivity.push_back(act);
        }
    }
    return true;
}

bool GitRepository::loadStatus(QString *error)
{
    QString out;
    if (!runOk({QStringLiteral("status"), QStringLiteral("--porcelain=2"), QStringLiteral("-z"),
                QStringLiteral("--untracked-files=all")},
               error, &out)) {
        // fallback porcelain v1
        if (!runOk({QStringLiteral("status"), QStringLiteral("--porcelain"), QStringLiteral("-uall")}, error, &out))
            return false;

        m_changes.clear();
        const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            if (line.size() < 4)
                continue;
            FileChange ch;
            const QChar x = line[0];
            const QChar y = line[1];
            QString path = line.mid(3);
            if (path.contains(QLatin1String(" -> ")))
                path = path.section(QLatin1String(" -> "), -1);
            ch.path = path;
            if (x == QLatin1Char('?') && y == QLatin1Char('?')) {
                ch.status = QStringLiteral("??");
                ch.staged = false;
            } else if (x != QLatin1Char(' ') && x != QLatin1Char('?')) {
                ch.status = QString(x);
                ch.staged = true;
                m_changes.push_back(ch);
                if (y != QLatin1Char(' ') && y != QLatin1Char('?')) {
                    FileChange un;
                    un.path = path;
                    un.status = QString(y);
                    un.staged = false;
                    m_changes.push_back(un);
                }
                continue;
            } else {
                ch.status = QString(y);
                ch.staged = false;
            }
            m_changes.push_back(ch);
        }
        return true;
    }

    // porcelain v2 with NUL
    m_changes.clear();
    const QByteArray raw = out.toUtf8();
    const QList<QByteArray> entries = raw.split('\0');
    for (const QByteArray &e : entries) {
        if (e.isEmpty())
            continue;
        const QString line = QString::fromUtf8(e);
        FileChange ch;
        if (line.startsWith(QLatin1Char('1')) || line.startsWith(QLatin1Char('2'))) {
            // 1 <XY> <sub> <mH> <mI> <mW> <hH> <hI> <path>
            // 2 <XY> ... <path>\t<orig>
            const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() < 9)
                continue;
            const QString xy = parts[1];
            QString path = parts.mid(8).join(QLatin1Char(' '));
            if (path.contains(QLatin1Char('\t')))
                path = path.section(QLatin1Char('\t'), 0, 0);
            const QChar x = xy.size() > 0 ? xy[0] : QLatin1Char('.');
            const QChar y = xy.size() > 1 ? xy[1] : QLatin1Char('.');
            if (x != QLatin1Char('.')) {
                FileChange staged;
                staged.path = path;
                staged.status = QString(x);
                staged.staged = true;
                m_changes.push_back(staged);
            }
            if (y != QLatin1Char('.')) {
                FileChange unstaged;
                unstaged.path = path;
                unstaged.status = QString(y);
                unstaged.staged = false;
                m_changes.push_back(unstaged);
            }
        } else if (line.startsWith(QLatin1Char('?'))) {
            ch.path = line.mid(2).trimmed();
            ch.status = QStringLiteral("??");
            ch.staged = false;
            m_changes.push_back(ch);
        } else if (line.startsWith(QLatin1Char('u'))) {
            const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 11) {
                ch.path = parts.mid(10).join(QLatin1Char(' '));
                ch.status = QStringLiteral("U");
                ch.staged = false;
                m_changes.push_back(ch);
            }
        }
    }
    return true;
}

bool GitRepository::loadRemotesTags(QString *error)
{
    QString out;
    m_remotes.clear();
    m_remoteDetails.clear();
    if (runOk({QStringLiteral("remote"), QStringLiteral("-v")}, error, &out)) {
        QSet<QString> seen;
        const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            // origin  https://... (fetch)
            const QString trimmed = line.trimmed();
            if (!trimmed.contains(QLatin1String("(fetch)")))
                continue;
            const QStringList parts = trimmed.split(QRegularExpression(QStringLiteral("\\s+")),
                                                    Qt::SkipEmptyParts);
            if (parts.size() < 2)
                continue;
            const QString name = parts[0];
            const QString url = parts[1];
            if (seen.contains(name))
                continue;
            seen.insert(name);
            m_remotes << name;
            QVariantMap entry;
            entry.insert(QStringLiteral("name"), name);
            entry.insert(QStringLiteral("url"), url);
            m_remoteDetails.push_back(entry);
        }
    }

    m_tags.clear();
    if (runOk({QStringLiteral("tag"), QStringLiteral("--list"), QStringLiteral("--sort=-creatordate")}, error, &out)) {
        const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &t : lines) {
            m_tags.push_back(t.trimmed());
            if (m_tags.size() >= 50)
                break;
        }
    }
    return true;
}

bool GitRepository::loadStats(QString *error)
{
    m_stats = {};
    m_stats.branchCount = 0;
    for (const BranchInfo &b : m_branches) {
        if (!b.remote)
            ++m_stats.branchCount;
    }
    m_stats.tagCount = m_tags.size();
    m_stats.changedFileCount = m_changes.size();
    m_stats.conflictCount = 0;
    for (const FileChange &c : m_changes) {
        if (c.status == QLatin1String("U") || c.status == QLatin1String("AA")
            || c.status == QLatin1String("DD") || c.status == QLatin1String("AU")
            || c.status == QLatin1String("UA") || c.status == QLatin1String("DU")
            || c.status == QLatin1String("UD")) {
            ++m_stats.conflictCount;
        }
    }

    QString out;
    if (runOk({QStringLiteral("rev-list"), QStringLiteral("--count"), QStringLiteral("--all")}, error, &out))
        m_stats.commitCount = out.trimmed().toInt();

    QSet<QString> authors;
    for (const CommitInfo &c : m_commits) {
        if (!c.authorEmail.isEmpty())
            authors.insert(c.authorEmail.toLower());
        else if (!c.authorName.isEmpty())
            authors.insert(c.authorName);
    }
    m_stats.contributorCount = authors.size();
    return true;
}

bool GitRepository::loadStashes(QString *error)
{
    m_stashes.clear();
    QString out;
    if (!runOk({QStringLiteral("stash"), QStringLiteral("list"),
                QStringLiteral("--format=%gd%x1f%gs")},
               error, &out)) {
        return true; // empty / no stash is fine
    }
    const QStringList lines = out.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QStringList parts = line.split(QChar(0x1f));
        QVariantMap entry;
        entry.insert(QStringLiteral("ref"), parts.value(0).trimmed());
        entry.insert(QStringLiteral("message"), parts.value(1).trimmed());
        // stash@{N}
        const QString ref = parts.value(0).trimmed();
        int idx = -1;
        const int l = ref.indexOf(QLatin1Char('{'));
        const int r = ref.indexOf(QLatin1Char('}'));
        if (l >= 0 && r > l)
            idx = ref.mid(l + 1, r - l - 1).toInt();
        entry.insert(QStringLiteral("index"), idx);
        m_stashes.push_back(entry);
        if (m_stashes.size() >= 30)
            break;
    }
    return true;
}

bool GitRepository::loadAheadBehind(QString *error)
{
    Q_UNUSED(error)
    m_ahead = 0;
    m_behind = 0;
    m_hasUpstream = false;
    if (m_path.isEmpty())
        return true;

    QString up;
    if (!runOk({QStringLiteral("rev-parse"), QStringLiteral("--abbrev-ref"),
                QStringLiteral("@{u}")},
               nullptr, &up)) {
        return true;
    }
    if (up.trimmed().isEmpty())
        return true;
    m_hasUpstream = true;

    QString counts;
    if (!runOk({QStringLiteral("rev-list"), QStringLiteral("--left-right"),
                QStringLiteral("--count"), QStringLiteral("@{u}...HEAD")},
               nullptr, &counts)) {
        return true;
    }
    // left = upstream-only (behind), right = HEAD-only (ahead)
    const QStringList parts = counts.trimmed().split(QRegularExpression(QStringLiteral("[\\s\\t]+")),
                                                     Qt::SkipEmptyParts);
    if (parts.size() >= 2) {
        m_behind = parts[0].toInt();
        m_ahead = parts[1].toInt();
    }
    return true;
}

bool GitRepository::isUntracked(const QString &path) const
{
    for (const FileChange &c : m_changes) {
        if (!c.staged && c.path == path && c.status == QLatin1String("??"))
            return true;
    }
    return false;
}

void GitRepository::buildProjectTree()
{
    m_projectTree.clear();
    QDir root(m_path);
    const QFileInfoList entries = root.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
                                                     QDir::DirsFirst | QDir::Name);
    QVariantList children;
    for (const QFileInfo &fi : entries) {
        if (fi.fileName() == QLatin1String(".git"))
            continue;
        QVariantMap node;
        node.insert(QStringLiteral("title"), fi.fileName());
        node.insert(QStringLiteral("icon"), fi.isDir() ? QStringLiteral("folder") : QStringLiteral("description"));
        node.insert(QStringLiteral("path"), fi.absoluteFilePath());
        children.push_back(node);
        if (children.size() >= 40)
            break;
    }
    QVariantMap project;
    project.insert(QStringLiteral("title"), name());
    project.insert(QStringLiteral("icon"), QStringLiteral("inventory_2"));
    project.insert(QStringLiteral("expanded"), true);
    project.insert(QStringLiteral("children"), children);
    m_projectTree.push_back(project);
}

QString GitRepository::fileDiff(const QString &path, bool staged) const
{
    if (m_path.isEmpty() || path.isEmpty())
        return {};
    QStringList args = {QStringLiteral("diff"), QStringLiteral("--no-color")};
    if (staged)
        args << QStringLiteral("--cached");
    args << QStringLiteral("--") << path;
    QString out;
    runOk(args, nullptr, &out);
    if (out.isEmpty() && !staged) {
        // untracked: show as full add
        QFile f(QDir(m_path).filePath(path));
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString content = QString::fromUtf8(f.read(200000));
            QString diff = QStringLiteral("--- /dev/null\n+++ b/%1\n").arg(path);
            for (const QString &line : content.split(QLatin1Char('\n')))
                diff += QLatin1Char('+') + line + QLatin1Char('\n');
            return diff;
        }
    }
    return out;
}

QString GitRepository::commitDiff(const QString &commitId) const
{
    if (m_path.isEmpty() || commitId.isEmpty())
        return {};
    QString out;
    runOk({QStringLiteral("show"), QStringLiteral("--stat"), QStringLiteral("--patch"),
           QStringLiteral("--format="), QStringLiteral("--no-color"), commitId},
          nullptr, &out);
    return out;
}

CommitInfo GitRepository::commitById(const QString &id) const
{
    for (const CommitInfo &c : m_commits) {
        if (c.id == id || c.id.startsWith(id) || c.shortId == id)
            return c;
    }
    return {};
}

QVariantList GitRepository::fileHistory(const QString &path, int limit) const
{
    QVariantList out;
    if (m_path.isEmpty() || path.trimmed().isEmpty())
        return out;
    const int n = qBound(1, limit, 100);
    QString raw;
    if (!runOk({QStringLiteral("log"),
                QStringLiteral("--pretty=format:%H%x1f%h%x1f%s%x1f%an%x1f%at"),
                QStringLiteral("-n"), QString::number(n),
                QStringLiteral("--"), path.trimmed()},
               nullptr, &raw)) {
        return out;
    }
    const QStringList lines = raw.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QStringList f = line.split(QChar(0x1f));
        if (f.size() < 5)
            continue;
        QVariantMap row;
        row.insert(QStringLiteral("id"), f[0]);
        row.insert(QStringLiteral("shortId"), f[1]);
        row.insert(QStringLiteral("subject"), f[2]);
        row.insert(QStringLiteral("author"), f[3]);
        row.insert(QStringLiteral("date"),
                   QDateTime::fromSecsSinceEpoch(f[4].toLongLong()).toString(QStringLiteral("yyyy-MM-dd HH:mm")));
        out.push_back(row);
    }
    return out;
}

QVariantList GitRepository::fileBlame(const QString &path) const
{
    QVariantList out;
    if (m_path.isEmpty() || path.trimmed().isEmpty())
        return out;
    QString raw;
    // porcelain: hash author ... then \tline
    if (!runOk({QStringLiteral("blame"), QStringLiteral("--porcelain"),
                QStringLiteral("--"), path.trimmed()},
               nullptr, &raw)) {
        return out;
    }
    const QStringList lines = raw.split(QLatin1Char('\n'));
    QString curHash;
    QString curAuthor;
    QString curDate;
    for (const QString &line : lines) {
        if (line.startsWith(QLatin1Char('\t'))) {
            QVariantMap row;
            row.insert(QStringLiteral("shortId"), curHash.left(7));
            row.insert(QStringLiteral("author"), curAuthor);
            row.insert(QStringLiteral("date"), curDate);
            row.insert(QStringLiteral("text"), line.mid(1));
            out.push_back(row);
            if (out.size() >= 2000)
                break;
            continue;
        }
        if (line.startsWith(QLatin1String("author "))) {
            curAuthor = line.mid(7);
            continue;
        }
        if (line.startsWith(QLatin1String("author-time "))) {
            curDate = QDateTime::fromSecsSinceEpoch(line.mid(12).toLongLong())
                          .toString(QStringLiteral("yyyy-MM-dd"));
            continue;
        }
        // header line: <40-hex> <orig> <final> [<num>]
        if (line.size() >= 40 && line[0].isLetterOrNumber()) {
            const QStringList parts = line.split(QLatin1Char(' '));
            if (!parts.isEmpty() && parts[0].size() >= 7)
                curHash = parts[0];
        }
    }
    return out;
}

QVariantMap GitRepository::compareRefs(const QString &baseRef, const QString &headRef, int limit) const
{
    QVariantMap result;
    result.insert(QStringLiteral("ahead"), 0);
    result.insert(QStringLiteral("behind"), 0);
    result.insert(QStringLiteral("commits"), QVariantList{});
    if (m_path.isEmpty() || baseRef.trimmed().isEmpty() || headRef.trimmed().isEmpty())
        return result;

    QString counts;
    const QString range = baseRef.trimmed() + QLatin1String("...") + headRef.trimmed();
    if (runOk({QStringLiteral("rev-list"), QStringLiteral("--left-right"),
               QStringLiteral("--count"), range},
              nullptr, &counts)) {
        const QStringList parts = counts.trimmed().split(QRegularExpression(QStringLiteral("[\\s\\t]+")),
                                                         Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            result.insert(QStringLiteral("behind"), parts[0].toInt()); // only in base
            result.insert(QStringLiteral("ahead"), parts[1].toInt());  // only in head
        }
    }

    QString raw;
    const int n = qBound(1, limit, 80);
    if (runOk({QStringLiteral("log"),
               QStringLiteral("--pretty=format:%H%x1f%h%x1f%s%x1f%an%x1f%at"),
               QStringLiteral("-n"), QString::number(n),
               headRef.trimmed(), QLatin1String("^") + baseRef.trimmed()},
              nullptr, &raw)) {
        QVariantList commits;
        for (const QString &line : raw.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            const QStringList f = line.split(QChar(0x1f));
            if (f.size() < 5)
                continue;
            QVariantMap row;
            row.insert(QStringLiteral("id"), f[0]);
            row.insert(QStringLiteral("shortId"), f[1]);
            row.insert(QStringLiteral("subject"), f[2]);
            row.insert(QStringLiteral("author"), f[3]);
            row.insert(QStringLiteral("date"),
                       QDateTime::fromSecsSinceEpoch(f[4].toLongLong())
                           .toString(QStringLiteral("yyyy-MM-dd HH:mm")));
            commits.push_back(row);
        }
        result.insert(QStringLiteral("commits"), commits);
    }
    return result;
}

bool GitRepository::stage(const QString &path, QString *error)
{
    return runOk({QStringLiteral("add"), QStringLiteral("--"), path}, error) && refresh(error);
}

bool GitRepository::unstage(const QString &path, QString *error)
{
    return runOk({QStringLiteral("restore"), QStringLiteral("--staged"), QStringLiteral("--"), path}, error)
           && refresh(error);
}

bool GitRepository::stageAll(QString *error)
{
    return runOk({QStringLiteral("add"), QStringLiteral("-A")}, error) && refresh(error);
}

bool GitRepository::unstageAll(QString *error)
{
    return runOk({QStringLiteral("restore"), QStringLiteral("--staged"), QStringLiteral(".")}, error)
           && refresh(error);
}

bool GitRepository::discard(const QString &path, QString *error)
{
    if (path.isEmpty()) {
        if (error)
            *error = QStringLiteral("Empty path");
        return false;
    }
    if (isUntracked(path)) {
        if (!runOk({QStringLiteral("clean"), QStringLiteral("-f"), QStringLiteral("--"), path}, error))
            return false;
    } else {
        if (!runOk({QStringLiteral("restore"), QStringLiteral("--worktree"), QStringLiteral("--"), path}, error))
            return false;
    }
    return refresh(error);
}

bool GitRepository::discardAll(QString *error)
{
    // Only tracked unstaged worktree changes — does not touch staged or delete untracked.
    if (!runOk({QStringLiteral("restore"), QStringLiteral("--worktree"), QStringLiteral(".")}, error))
        return false;
    return refresh(error);
}

bool GitRepository::stashSave(const QString &message, QString *error)
{
    QStringList args = {QStringLiteral("stash"), QStringLiteral("push"), QStringLiteral("-u")};
    if (!message.trimmed().isEmpty())
        args << QStringLiteral("-m") << message.trimmed();
    return runOk(args, error) && refresh(error);
}

bool GitRepository::stashPop(QString *error)
{
    return runOk({QStringLiteral("stash"), QStringLiteral("pop")}, error) && refresh(error);
}

bool GitRepository::stashDrop(int index, QString *error)
{
    if (index < 0) {
        if (error)
            *error = QStringLiteral("Invalid stash index");
        return false;
    }
    const QString ref = QStringLiteral("stash@{%1}").arg(index);
    return runOk({QStringLiteral("stash"), QStringLiteral("drop"), ref}, error) && refresh(error);
}

bool GitRepository::commit(const QString &message, bool amend, QString *error)
{
    if (message.trimmed().isEmpty() && !amend) {
        if (error)
            *error = QStringLiteral("Commit message is empty");
        return false;
    }
    QStringList args = {QStringLiteral("commit")};
    if (amend)
        args << QStringLiteral("--amend");
    if (!message.trimmed().isEmpty())
        args << QStringLiteral("-m") << message;
    else if (amend)
        args << QStringLiteral("--no-edit");
    return runOk(args, error) && refresh(error);
}

bool GitRepository::checkout(const QString &ref, QString *error)
{
    return runOk({QStringLiteral("switch"), QStringLiteral("--"), ref}, error) && refresh(error);
}

bool GitRepository::createBranch(const QString &name, QString *error)
{
    return runOk({QStringLiteral("branch"), name}, error) && refresh(error);
}

bool GitRepository::createBranchAt(const QString &name, const QString &startPoint, QString *error)
{
    if (name.trimmed().isEmpty() || startPoint.trimmed().isEmpty()) {
        if (error)
            *error = QStringLiteral("Branch name and start point required");
        return false;
    }
    return runOk({QStringLiteral("branch"), name.trimmed(), startPoint.trimmed()}, error)
           && refresh(error);
}

bool GitRepository::deleteBranch(const QString &name, bool force, QString *error)
{
    return runOk({QStringLiteral("branch"),
                  force ? QStringLiteral("-D") : QStringLiteral("-d"),
                  name},
                 error)
           && refresh(error);
}

bool GitRepository::merge(const QString &branch, QString *error)
{
    return runOk({QStringLiteral("merge"), QStringLiteral("--no-edit"), branch}, error) && refresh(error);
}

bool GitRepository::revertCommit(const QString &commitId, QString *error)
{
    if (commitId.trimmed().isEmpty()) {
        if (error)
            *error = QStringLiteral("Commit id required");
        return false;
    }
    return runOk({QStringLiteral("revert"), QStringLiteral("--no-edit"), commitId.trimmed()}, error)
           && refresh(error);
}

bool GitRepository::cherryPick(const QString &commitId, QString *error)
{
    if (commitId.trimmed().isEmpty()) {
        if (error)
            *error = QStringLiteral("Commit id required");
        return false;
    }
    return runOk({QStringLiteral("cherry-pick"), QStringLiteral("--ff"), commitId.trimmed()}, error)
           && refresh(error);
}

bool GitRepository::resetTo(const QString &commitId, const QString &mode, QString *error)
{
    if (commitId.trimmed().isEmpty()) {
        if (error)
            *error = QStringLiteral("Commit id required");
        return false;
    }
    QString flag = QStringLiteral("--mixed");
    const QString m = mode.trimmed().toLower();
    if (m == QLatin1String("soft"))
        flag = QStringLiteral("--soft");
    else if (m == QLatin1String("hard"))
        flag = QStringLiteral("--hard");
    else
        flag = QStringLiteral("--mixed");
    return runOk({QStringLiteral("reset"), flag, commitId.trimmed()}, error) && refresh(error);
}

bool GitRepository::fetch(QString *error)
{
    return runOk({QStringLiteral("fetch"), QStringLiteral("--all"), QStringLiteral("--prune")}, error, nullptr)
           && refresh(error);
}

bool GitRepository::pull(QString *error)
{
    return runOk({QStringLiteral("pull"), QStringLiteral("--ff-only")}, error) && refresh(error);
}

bool GitRepository::pullRebase(QString *error)
{
    return runOk({QStringLiteral("pull"), QStringLiteral("--rebase"), QStringLiteral("--autostash")}, error)
           && refresh(error);
}

bool GitRepository::push(QString *error)
{
    return runOk({QStringLiteral("push")}, error) && refresh(error);
}

bool GitRepository::pushSetUpstream(QString *error)
{
    QString branch = m_currentBranch;
    if (branch.isEmpty() || branch == QLatin1String("(detached)")) {
        if (error)
            *error = QStringLiteral("No local branch to push");
        return false;
    }
    QString remote = QStringLiteral("origin");
    if (!m_remotes.isEmpty())
        remote = m_remotes.first();
    return runOk({QStringLiteral("push"), QStringLiteral("-u"), remote, branch}, error)
           && refresh(error);
}

bool GitRepository::createTag(const QString &name, const QString &message, QString *error)
{
    if (name.trimmed().isEmpty()) {
        if (error)
            *error = QStringLiteral("Tag name is empty");
        return false;
    }
    QStringList args = {QStringLiteral("tag")};
    if (!message.trimmed().isEmpty())
        args << QStringLiteral("-a") << name.trimmed() << QStringLiteral("-m") << message.trimmed();
    else
        args << name.trimmed();
    return runOk(args, error) && refresh(error);
}

bool GitRepository::deleteTag(const QString &name, QString *error)
{
    if (name.trimmed().isEmpty()) {
        if (error)
            *error = QStringLiteral("Tag name is empty");
        return false;
    }
    return runOk({QStringLiteral("tag"), QStringLiteral("-d"), name.trimmed()}, error) && refresh(error);
}

bool GitRepository::cloneRepo(const QString &url, const QString &destDir, QString *error)
{
    const QString cleanedUrl = url.trimmed();
    const QString dest = QDir::cleanPath(destDir);
    if (cleanedUrl.isEmpty() || dest.isEmpty()) {
        if (error)
            *error = QStringLiteral("URL and destination are required");
        return false;
    }
    if (QFileInfo::exists(dest)) {
        if (error)
            *error = QStringLiteral("Destination already exists");
        return false;
    }
    const QFileInfo destInfo(dest);
    const QString parent = destInfo.absolutePath();
    if (!QDir(parent).exists()) {
        if (error)
            *error = QStringLiteral("Parent directory does not exist");
        return false;
    }

    const GitResult r = m_runner->run(QString(),
                                      {QStringLiteral("clone"), cleanedUrl, dest},
                                      600000);
    if (!r.ok()) {
        if (error) {
            *error = r.stderrText.trimmed();
            if (error->isEmpty())
                *error = QStringLiteral("Clone failed (%1)").arg(r.exitCode);
        }
        return false;
    }
    return open(dest, error);
}

bool GitRepository::initRepo(const QString &path, QString *error)
{
    const QString cleaned = QDir::cleanPath(path);
    if (cleaned.isEmpty() || !QFileInfo::exists(cleaned) || !QFileInfo(cleaned).isDir()) {
        if (error)
            *error = QStringLiteral("Directory does not exist");
        return false;
    }
    const GitResult r = m_runner->run(cleaned, {QStringLiteral("init")});
    if (!r.ok()) {
        if (error) {
            *error = r.stderrText.trimmed();
            if (error->isEmpty())
                *error = QStringLiteral("git init failed (%1)").arg(r.exitCode);
        }
        return false;
    }
    return open(cleaned, error);
}

bool GitRepository::abortMerge(QString *error)
{
    if (!m_mergeInProgress && !QFileInfo::exists(QDir(m_path).filePath(QStringLiteral(".git/MERGE_HEAD")))) {
        if (error)
            *error = QStringLiteral("No merge in progress");
        return false;
    }
    return runOk({QStringLiteral("merge"), QStringLiteral("--abort")}, error) && refresh(error);
}

bool GitRepository::abortRebase(QString *error)
{
    return runOk({QStringLiteral("rebase"), QStringLiteral("--abort")}, error) && refresh(error);
}

bool GitRepository::continueRebase(QString *error)
{
    return runOk({QStringLiteral("rebase"), QStringLiteral("--continue")}, error) && refresh(error);
}

bool GitRepository::resolveConflict(const QString &path, const QString &side, QString *error)
{
    if (path.trimmed().isEmpty()) {
        if (error)
            *error = QStringLiteral("Path required");
        return false;
    }
    const QString s = side.trimmed().toLower();
    const QString which = (s == QLatin1String("theirs"))
                              ? QStringLiteral("--theirs")
                              : QStringLiteral("--ours");
    if (!runOk({QStringLiteral("checkout"), which, QStringLiteral("--"), path.trimmed()}, error))
        return false;
    return runOk({QStringLiteral("add"), QStringLiteral("--"), path.trimmed()}, error) && refresh(error);
}
