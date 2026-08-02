#include "GraphNodeModel.h"

#include <QSet>

GraphNodeModel::GraphNodeModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int GraphNodeModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QColor GraphNodeModel::colorForColumn(int col)
{
    static const QColor palette[] = {
        QColor(QStringLiteral("#4FC3F7")),
        QColor(QStringLiteral("#81C784")),
        QColor(QStringLiteral("#FFB74D")),
        QColor(QStringLiteral("#E57373")),
        QColor(QStringLiteral("#BA68C8")),
        QColor(QStringLiteral("#4DB6AC")),
        QColor(QStringLiteral("#FFD54F")),
        QColor(QStringLiteral("#90A4AE")),
    };
    return palette[col % 8];
}

QVariant GraphNodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};
    const GraphNode &n = m_items.at(index.row());
    switch (role) {
    case IdRole: return n.id;
    case ShortIdRole: return n.shortId;
    case SubjectRole: return n.subject;
    case AuthorRole: return n.author;
    case DateRole: return n.date;
    case RefsRole: return n.refs;
    case ColumnRole: return n.column;
    case RowRole: return n.row;
    case ColorRole: return n.color;
    case ParentsRole: return n.parents;
    case ParentColumnsRole: {
        QVariantList cols;
        for (int c : n.parentColumns)
            cols.push_back(c);
        return cols;
    }
    default: return {};
    }
}

QHash<int, QByteArray> GraphNodeModel::roleNames() const
{
    return {
        {IdRole, "commitId"},
        {ShortIdRole, "shortId"},
        {SubjectRole, "subject"},
        {AuthorRole, "author"},
        {DateRole, "date"},
        {RefsRole, "refs"},
        {ColumnRole, "column"},
        {RowRole, "row"},
        {ColorRole, "nodeColor"},
        {ParentsRole, "parents"},
        {ParentColumnsRole, "parentColumns"},
    };
}

void GraphNodeModel::rebuild(const QList<CommitInfo> &commits)
{
    beginResetModel();
    m_items.clear();
    m_maxColumn = 0;

    // Simplified lane assignment: first-parent prefers column 0, side parents open new lanes.
    QHash<QString, int> assigned;
    QVector<QString> lanes; // lane index → tip commit currently occupying

    int row = 0;
    for (const CommitInfo &c : commits) {
        GraphNode node;
        node.id = c.id;
        node.shortId = c.shortId;
        node.subject = c.subject;
        node.author = c.authorName;
        node.date = c.authorDate.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
        node.refs = c.refs;
        node.parents = c.parents;
        node.row = row;

        int col = -1;
        for (int i = 0; i < lanes.size(); ++i) {
            if (lanes[i] == c.id) {
                col = i;
                break;
            }
        }
        if (col < 0) {
            col = lanes.size();
            lanes.push_back(c.id);
        }

        node.column = col;
        node.color = colorForColumn(col);
        assigned.insert(c.id, col);

        // Place parents into lanes
        for (int pi = 0; pi < c.parents.size(); ++pi) {
            const QString &parent = c.parents[pi];
            int pcol = -1;
            if (assigned.contains(parent)) {
                pcol = assigned.value(parent);
            } else if (pi == 0) {
                // first parent continues this lane
                lanes[col] = parent;
                pcol = col;
            } else {
                // find free lane or append
                pcol = -1;
                for (int i = 0; i < lanes.size(); ++i) {
                    if (lanes[i].isEmpty()) {
                        pcol = i;
                        lanes[i] = parent;
                        break;
                    }
                }
                if (pcol < 0) {
                    pcol = lanes.size();
                    lanes.push_back(parent);
                }
            }
            node.parentColumns.push_back(pcol >= 0 ? pcol : col);
        }

        // Clear lane if this commit was a tip and no first-parent continuation
        if (c.parents.isEmpty())
            lanes[col].clear();

        m_maxColumn = qMax(m_maxColumn, node.column);
        for (int pc : node.parentColumns)
            m_maxColumn = qMax(m_maxColumn, pc);

        m_items.push_back(node);
        ++row;
    }

    endResetModel();
    emit graphLayoutChanged();
}

void GraphNodeModel::clear()
{
    beginResetModel();
    m_items.clear();
    m_maxColumn = 0;
    endResetModel();
    emit graphLayoutChanged();
}
