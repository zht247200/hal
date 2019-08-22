#include "gui/netlist_model/gate_netlist_item.h"

#include "netlist/gate.h"

#include "gui/gui_globals.h"

gate_netlist_item::gate_netlist_item(const u32 id) : netlist_item(hal::item_type::gate, id, QString::fromStdString(g_netlist->get_gate_by_id(id)->get_name()))
{

}
