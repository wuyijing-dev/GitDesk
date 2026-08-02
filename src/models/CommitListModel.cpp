#include "CommitListModel.h"

CommitListModel::CommitListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CommitListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant CommitListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};
    const CommitInfo &c = m_items.at(index.row());
    switch (role) {
    case IdRole: return c.id;
    case ShortIdRole: return c.shortId;
    case SubjectRole: return c.subject;
    case BodyRole: return c.body;
    case AuthorRole: return c.authorName;
    case EmailRole: return c.authorEmail;
    case DateRole: return c.authorDate.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
    case RefsRole: return c.refs;
    case ParentsRole: return c.parents;
    default: return {};
    }
}

QHash<int, QByteArray> CommitListModel::roleNames() const
{
    return {
        {IdRole, "commitId"},
        {ShortIdRole, "shortId"},
        {SubjectRole, "subject"},
        {BodyRole, "body"},
        {AuthorRole, "author"},
        {EmailRole, "email"},
        {DateRole, "date"},
        {RefsRole, "refs"},
        {ParentsRole, "parents"},
    };
}

void CommitListModel::setCommits(const QList<CommitInfo> &commits)
{
    beginResetModel();
    m_items = QVector<CommitInfo>(commits.cbegin(), commits.cend());
    endResetModel();
}

void CommitListModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}
