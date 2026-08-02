#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "GitRepository.h"

class FileChangeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        StatusRole,
        StagedRole,
        AdditionsRole,
        DeletionsRole,
        DisplayStatusRole
    };

    explicit FileChangeModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setChanges(const QList<FileChange> &changes);
    void clear();

private:
    static QString displayStatus(const FileChange &c);
    QVector<FileChange> m_items;
};
