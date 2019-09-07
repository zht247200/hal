#include "gui/netlist_model/module_netlist_item.h"

#include "netlist/module.h"

#include "gui/gui_globals.h"

module_netlist_item::module_netlist_item(const u32 id) : netlist_item(hal::item_type::module, id, QString::fromStdString(g_netlist->get_module_by_id(id)->get_name()))
{
    // SET COLOR
}

module_netlist_item::module_netlist_item(const u32 id, const QString& name) : netlist_item(hal::item_type::module, id, name)
{
    // THIS CONSTRUCTOR IS ONLY USED FOR THE ROOT ITEM
    // SET COLOR
}

void module_netlist_item::insert_child(const int row, netlist_item* child)
{
    m_child_items.insert(row, child);
}

void module_netlist_item::remove_child(netlist_item* child)
{
    m_child_items.removeOne(child);
}

const QList<netlist_item*> &module_netlist_item::child_items() const
{
    return m_child_items;
}

netlist_item* module_netlist_item::child(const int row) const
{
    return m_child_items.value(row);
}

const netlist_item* module_netlist_item::const_child(const int row) const
{
    return m_child_items.value(row);
}

int module_netlist_item::childCount() const
{
    return m_child_items.count();
}

int module_netlist_item::row() const
{
    if (m_parent)
        return m_parent->m_child_items.indexOf(static_cast<netlist_item*>(const_cast<module_netlist_item*>(this)));
    else
        return 0;
}

void module_netlist_item::set_color(const QColor& color)
{
    m_color = color;
}

QColor module_netlist_item::color() const
{
    return m_color;
}
