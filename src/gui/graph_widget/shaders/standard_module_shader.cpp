#include "gui/graph_widget/shaders/standard_module_shader.h"

#include "gui/graph_widget/contexts/module_context.h"
#include "gui/gui_globals.h"

#include "netlist/module.h"

bool standard_module_shader::s_color_gates = true; // SET VIA SETTING

standard_module_shader::standard_module_shader(const module_context* const context) : module_shader(context)
{

}

void standard_module_shader::update()
{
    m_shading.module_visuals.clear();
    m_shading.gate_visuals.clear();
    m_shading.net_visuals.clear();

    for (u32 id : m_context->modules())
    {
        graphics_node::visuals v;
        v.main_color = g_netlist_relay.get_module_color(id);
        m_shading.module_visuals.insert(id, v);
    }

    if (s_color_gates)
    {
        for (u32 id : m_context->gates())
        {
            std::shared_ptr<gate> g = g_netlist->get_gate_by_id(id);
            assert(g);

            std::shared_ptr<module> m = g->get_module();
            assert(m);

            graphics_node::visuals v;
            v.main_color = g_netlist_relay.get_module_color(m->get_id());
            m_shading.gate_visuals.insert(id, v);
        }
    }
}
