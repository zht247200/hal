#include "gui/graph_widget/contexts/cone_context.h"

#include "gui/gui_globals.h"

cone_context::cone_context(const QString& name) : graph_context(type::dynamic, g_graph_context_manager.get_default_layouter(this), g_graph_context_manager.get_default_shader(this)),
    m_name(name)
{

}

bool cone_context::contains_module(const u32 id) const
{
    // NOT YET IMPLEMENTED
    return false;
}

bool cone_context::contains_gate(const u32 id) const
{
    // NOT YET IMPLEMENTED
    return false;
}

bool cone_context::contains_net(const u32 id) const
{
    // NOT YET IMPLEMENTED
    return false;
}

QString cone_context::name() const
{
    return m_name;
}

void cone_context::apply_changes()
{
    assert(m_unapplied_changes);
    // APPLY STUFF
    m_unapplied_changes = false;
    schedule_relayout();
}
