#include "FileChangeModel.h"

FileChangeModel::FileChangeModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int FileChangeModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QString FileChangeModel::displayStatus(const FileChange &c)
{
    if (c.status == QLatin1String("??"))
        return QStringLiteral("Untracked");
    if (c.status == QLatin1String("A"))
        return QStringLiteral("Added");
    if (c.status == QLatin1String("D"))
        return QStringLiteral("Deleted");
    if (c.status == QLatin1String("R"))
        return QStringLiteral("Renamed");
    if (c.status == QLatin1String("U"))
        return QStringLiteral("Conflict");
    return QStringLiteral("Modified");
}

QVariant FileChangeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};
    const FileChange &c = m_items.at(index.row());
    switch (role) {
    case PathRole: return c.path;
    case StatusRole: return c.status;
    case StagedRole: return c.staged;
    case AdditionsRole: return c.additions;
    case DeletionsRole: return c.deletions;
    case DisplayStatusRole: return displayStatus(c);
    default: return {};
    }
}

QHash<int, QByteArray> FileChangeModel::roleNames() const
{
    return {
        {PathRole, "path"},
        {StatusRole, "status"},
        {StagedRole, "staged"},
        {AdditionsRole, "additions"},
        {DeletionsRole, "deletions"},
        {DisplayStatusRole, "displayStatus"},
    };
}

void FileChangeModel::setChanges(const QList<FileChange> &changes)
{
    beginResetModel();
    m_items = QVector<FileChange>(changes.cbegin(), changes.cend());
    endResetModel();
}

void FileChangeModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}
