#ifndef MODULE_NETLIST_ITEM_H
#define MODULE_NETLIST_ITEM_H

#include "gui/netlist_model/netlist_item.h"

class module_netlist_item : public netlist_item
{
public:
    module_netlist_item(const u32 id);
    module_netlist_item(const u32 id, const QString& name);

    void insert_child(const int row, netlist_item* child);
    void remove_child(netlist_item* child);

    void set_color(const QColor& color);

    const QList<netlist_item*>& child_items() const;

    netlist_item* child(const int row) const;
    const netlist_item* const_child(const int row) const;

    int childCount() const;

    int row() const;

    QColor color() const;

private:
    QList<netlist_item*> m_child_items;

    QColor m_color;
};

#endif // MODULE_NETLIST_ITEM_H
