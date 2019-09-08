#include "gui/netlist_model/netlist_model.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/gui_utility.h"
#include "gui/netlist_model/gate_netlist_item.h"
#include "gui/netlist_model/module_netlist_item.h"
#include "gui/netlist_model/net_netlist_item.h"

netlist_model::netlist_model(QObject* parent) : QAbstractItemModel(parent)
{
    m_top_module_item = new module_netlist_item(0, ""); // USE TOP ITEM AS ROOT ???
}

netlist_model::~netlist_model()
{
    delete m_top_module_item;
}

QModelIndex netlist_model::index(int row, int column, const QModelIndex& parent) const
{
    // BEHAVIOR FOR ILLEGAL INDICES IS UNDEFINED
    // SEE QT DOCUMENTATION

    if (column != 0 || parent.column() != 0)
        return QModelIndex();

    netlist_item* parent_item = get_item(parent);

    if (parent_item->type() != hal::item_type::module) // CAN THIS BE ASSERTED ?
        return QModelIndex();

    netlist_item* child_item = static_cast<module_netlist_item*>(parent_item)->child(row);

    if (child_item)
        return createIndex(row, column, child_item);
    else
        return QModelIndex();
}

QModelIndex netlist_model::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    netlist_item* child_item  = get_item(index);
    netlist_item* parent_item = child_item->parent();

    if (parent_item == m_top_module_item)
        return QModelIndex();

    return createIndex(parent_item->row(), 0, parent_item);
}

int netlist_model::rowCount(const QModelIndex& parent) const
{
    if (parent.column() != 0)
        return 0;

    netlist_item* parent_item = get_item(parent);

    if (parent_item->type() == hal::item_type::module)
        return static_cast<module_netlist_item*>(parent_item)->childCount();
    else
        return 0;
}

int netlist_model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return 1;
}

QVariant netlist_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    netlist_item* item = static_cast<netlist_item*>(index.internalPointer());

    if (!item)
        return QVariant();

    switch (role)
    {
    case Qt::DecorationRole:
    {
        if (item->type() == hal::item_type::module)
        {
            if (index.column() == 0)
            {
                QString run_icon_style = "all->" + static_cast<module_netlist_item*>(item)->color().name();
                QString run_icon_path  = ":/icons/filled-circle";

                return gui_utility::get_styled_svg_icon(run_icon_style, run_icon_path);
            }
        }

        return QVariant();
    }
    case Qt::DisplayRole:
    {
        return item->data(index.column());
    }
    case Qt::ForegroundRole:
    {
        if (item->highlighted())
            return QColor(QColor(255, 221, 0)); // USE STYLESHEETS
        else
            return QColor(QColor(255, 255, 255)); // USE STYLESHEETS
    }
    default: return QVariant();
    }

//    if (role == Qt::UserRole && index.column() == 0)
//    {
//        module_item* item = static_cast<module_item*>(index.internalPointer());
//        return QVariant::fromValue(item->id());
//    }

//    if (role == Qt::UserRole + 1 && index.column() == 0)
//    {
//        // PROBABLY OBSOLETE
//        module_item* item = static_cast<module_item*>(index.internalPointer());
//        return QVariant();
//    }

}

Qt::ItemFlags netlist_model::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant netlist_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

netlist_item* netlist_model::get_item(const QModelIndex& index) const
{
    if (index.isValid())
    {
        netlist_item* item = static_cast<netlist_item*>(index.internalPointer());

        if (item)
            return item;
    }

    return m_top_module_item;
}

QModelIndex netlist_model::get_index(const netlist_item* const item) const
{
    QVector<int> row_numbers;
    const netlist_item* current_item = item;

    while (current_item != m_top_module_item)
    {
        if (!current_item)
            return QModelIndex(); // SHOULD NEVER BE REACHED

        row_numbers.append(current_item->row());
        current_item = current_item->const_parent();
    }

    QModelIndex model_index = QModelIndex();

    for (QVector<int>::const_reverse_iterator i = row_numbers.crbegin(); i != row_numbers.crend(); ++i)
        model_index = index(*i, 0, model_index);

    return model_index;
}

void netlist_model::add_top_module()
{
    module_netlist_item* item = new module_netlist_item(1);
    item->set_parent(m_top_module_item);
    item->set_color(QColor(96, 110, 112));

    //QModelIndex index = get_index(m_root_item);

    beginInsertRows(index(0, 0, QModelIndex()), 0, 0);
    m_top_module_item->insert_child(0, item);
    endInsertRows();
}

void netlist_model::add_module(const u32 id, const u32 parent_module)
{
    assert(g_netlist->get_module_by_id(id));
    assert(g_netlist->get_module_by_id(parent_module));
    assert(!m_module_items.contains(id));
    assert(m_module_items.contains(parent_module));

    module_netlist_item* item = new module_netlist_item(id);
    module_netlist_item* parent = m_module_items.value(parent_module);

    item->set_parent(parent);

    // INSERT BY CATEGORY
    QModelIndex index = get_index(parent);

    int row = 0;

    while (row < parent->childCount())
    {
        if (item->name() < parent->child(row)->name())
            break;
        else
            ++row;
    }

    beginInsertRows(index, row, row);
    parent->insert_child(row, item);
    endInsertRows();

    m_module_items.insert(id, item);
}

void netlist_model::remove_module(const u32 id)
{
    assert(g_netlist->get_module_by_id(id));
    assert(m_module_items.contains(id));
    assert(id != 1);

    module_netlist_item* item = m_module_items.value(id);
    module_netlist_item* parent = item->parent();
    assert(parent);

    QModelIndex index = get_index(parent);

    int row = item->row();

    beginRemoveRows(index, row, row);
    parent->remove_child(item);
    endRemoveRows();

    m_module_items.remove(id);
}
