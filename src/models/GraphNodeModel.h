#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QHash>
#include <QColor>

#include "GitRepository.h"

struct GraphNode {
    QString id;
    QString shortId;
    QString subject;
    QString author;
    QString date;
    QStringList refs;
    QStringList parents;
    int column = 0;
    int row = 0;
    QColor color;
    QVector<int> parentColumns;
};

class GraphNodeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int maxColumn READ maxColumn NOTIFY graphLayoutChanged)
    Q_PROPERTY(int rowCountValue READ rowCountValue NOTIFY graphLayoutChanged)
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        ShortIdRole,
        SubjectRole,
        AuthorRole,
        DateRole,
        RefsRole,
        ColumnRole,
        RowRole,
        ColorRole,
        ParentsRole,
        ParentColumnsRole
    };

    explicit GraphNodeModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int maxColumn() const { return m_maxColumn; }
    int rowCountValue() const { return m_items.size(); }

    void rebuild(const QList<CommitInfo> &commits);
    void clear();

signals:
    void graphLayoutChanged();

private:
    static QColor colorForColumn(int col);
    QVector<GraphNode> m_items;
    int m_maxColumn = 0;
};
