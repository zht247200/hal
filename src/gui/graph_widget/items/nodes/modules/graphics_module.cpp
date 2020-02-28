#include "gui/graph_widget/items/nodes/modules/graphics_module.h"

#include "netlist/module.h"
#include "netlist/net.h"

graphics_module::graphics_module(const std::shared_ptr<const module> m) : graphics_node(hal::item_type::module, m->get_id(), QString::fromStdString(m->get_name()))
{
    int i = 1;
    for (const std::shared_ptr<net>& n : m->get_input_nets())
    {
        m_input_pins.append(module_pin{"I(" + QString::number(i) + ")", n->get_id()});
        ++i;
    }

    i = 1;
    for (const std::shared_ptr<net>& n : m->get_output_nets())
    {
        m_output_pins.append(module_pin{"(" + QString::number(i) + ")O", n->get_id()});
        ++i;
    }
}
