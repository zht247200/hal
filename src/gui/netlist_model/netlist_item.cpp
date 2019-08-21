#include "gui/netlist_model/netlist_item.h"

#include "gui/gui_utility.h"

netlist_item::netlist_item(const QString& name, const u32 id) :
    m_parent(nullptr),
    m_name(name),
    m_id(id),
    m_color(gui_utility::get_random_color()), // UNSURE, MAYBE A FIXED COLOR PALETTE OR DEFAULT COLOR IS BETTER
    m_highlighted(false)
{
}

netlist_item::~netlist_item()
{
    // UNSURE ABOUT THIS
    for (const netlist_item* const item : m_child_items)
        delete item;
}

void netlist_item::insert_child(int row, netlist_item* child)
{   
    m_child_items.insert(row, child);
}

void netlist_item::remove_child(netlist_item* child)
{
    m_child_items.removeOne(child);
}

netlist_item* netlist_item::parent()
{
    return m_parent;
}

netlist_item* netlist_item::child(int row)
{
    return m_child_items.value(row);
}

const netlist_item* netlist_item::const_parent() const
{
    return m_parent;
}

const netlist_item* netlist_item::const_child(int row) const
{
    return m_child_items.value(row);
}

int netlist_item::childCount() const
{
    return m_child_items.count();
}

int netlist_item::row() const
{
    if (m_parent)
        return m_parent->m_child_items.indexOf(const_cast<netlist_item*>(this));
    else
        return 0;
}

QVariant netlist_item::data(int column) const
{
    // DEBUG CODE, USE STYLED DELEGATES OR SOMETHING
    if (column != 0)
        return QVariant();
    return m_name;
}

QString netlist_item::name() const
{
    return m_name;
}

u32 netlist_item::id() const
{
    return m_id;
}

QColor netlist_item::color() const
{
    return m_color;
}

bool netlist_item::highlighted() const
{
    return m_highlighted;
}

void netlist_item::set_parent(netlist_item* parent)
{
    m_parent = parent;
}

void netlist_item::set_name(const QString& name)
{
    m_name = name;
}

void netlist_item::set_color(const QColor& color)
{
    m_color = color;
}

void netlist_item::set_highlighted(const bool highlighted)
{
    m_highlighted = highlighted;
}
