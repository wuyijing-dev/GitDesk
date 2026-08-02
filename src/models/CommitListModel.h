#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "GitRepository.h"

class CommitListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        ShortIdRole,
        SubjectRole,
        BodyRole,
        AuthorRole,
        EmailRole,
        DateRole,
        RefsRole,
        ParentsRole
    };

    explicit CommitListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setCommits(const QList<CommitInfo> &commits);
    void clear();

private:
    QVector<CommitInfo> m_items;
};
