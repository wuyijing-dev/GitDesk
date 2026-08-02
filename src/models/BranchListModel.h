#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "GitRepository.h"

class BranchListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        CurrentRole,
        RemoteRole,
        UpstreamRole,
        TipRole
    };

    explicit BranchListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setBranches(const QList<BranchInfo> &branches);
    void clear();

private:
    QVector<BranchInfo> m_items;
};
