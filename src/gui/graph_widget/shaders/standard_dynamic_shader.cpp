#include "gui/graph_widget/shaders/standard_dynamic_shader.h"

#include "gui/graph_widget/contexts/dynamic_context.h"
#include "gui/gui_globals.h"

#include "netlist/module.h"

bool standard_dynamic_shader::s_color_gates = false; // SET VIA SETTING

standard_dynamic_shader::standard_dynamic_shader(const dynamic_context* const context) : dynamic_shader(context)
{

}

void standard_dynamic_shader::update()
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
