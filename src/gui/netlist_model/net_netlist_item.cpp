#include "gui/netlist_model/net_netlist_item.h"

#include "netlist/net.h"

#include "gui/gui_globals.h"

net_netlist_item::net_netlist_item(const u32 id) : netlist_item(hal::item_type::module, id, QString::fromStdString(g_netlist->get_net_by_id(id)->get_name()))
{

}
