#include "gui/netlist_model/netlist_item.h"

#include "gui/gui_utility.h"
#include "gui/netlist_model/module_netlist_item.h"

netlist_item::netlist_item(const hal::item_type type, const u32 id, const QString& name) :
    m_parent(nullptr),
    m_item_type(type),
    m_id(id),
    m_name(name),
    m_highlighted(false)
{

}

module_netlist_item* netlist_item::parent() const
{
    return m_parent;
}

const module_netlist_item* netlist_item::const_parent() const
{
    return m_parent;
}

QVariant netlist_item::data(int column) const
{
    // DEBUG CODE, USE STYLED DELEGATES OR SOMETHING
    if (column != 0)
        return QVariant();
    return m_name;
}

int netlist_item::row() const
{
    if (m_parent)
    {
        return m_parent->child_items().indexOf(const_cast<netlist_item*>(this));

//        switch (m_item_type)
//        {
//        case hal::item_type::module: return m_parent->child_items().indexOf(const_cast<netlist_item*>(this));
//        case hal::item_type::gate: return m_parent->child_items().indexOf(const_cast<netlist_item*>(this));
//        case hal::item_type::net: return m_parent->child_items().indexOf(const_cast<netlist_item*>(this));
//        }
    }
    else
        return 0;
}

hal::item_type netlist_item::type() const
{
    return m_item_type;
}

QString netlist_item::name() const
{
    return m_name;
}

u32 netlist_item::id() const
{
    return m_id;
}

bool netlist_item::highlighted() const
{
    return m_highlighted;
}

void netlist_item::set_parent(module_netlist_item* parent)
{
    m_parent = parent;
}

void netlist_item::set_name(const QString& name)
{
    m_name = name;
}

void netlist_item::set_highlighted(const bool highlighted)
{
    m_highlighted = highlighted;
}
