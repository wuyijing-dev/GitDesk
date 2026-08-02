#include "BranchListModel.h"

BranchListModel::BranchListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int BranchListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant BranchListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};
    const BranchInfo &b = m_items.at(index.row());
    switch (role) {
    case NameRole: return b.name;
    case CurrentRole: return b.current;
    case RemoteRole: return b.remote;
    case UpstreamRole: return b.upstream;
    case TipRole: return b.tip;
    default: return {};
    }
}

QHash<int, QByteArray> BranchListModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {CurrentRole, "current"},
        {RemoteRole, "remote"},
        {UpstreamRole, "upstream"},
        {TipRole, "tip"},
    };
}

void BranchListModel::setBranches(const QList<BranchInfo> &branches)
{
    beginResetModel();
    m_items = QVector<BranchInfo>(branches.cbegin(), branches.cend());
    endResetModel();
}

void BranchListModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}
