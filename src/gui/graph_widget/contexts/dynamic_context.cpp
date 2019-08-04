#include "gui/graph_widget/contexts/dynamic_context.h"

#include "gui/gui_globals.h"

dynamic_context::dynamic_context(const QString& name) : graph_context(type::dynamic, g_graph_context_manager.get_default_layouter(this), g_graph_context_manager.get_default_shader(this)),
    m_name(name)
{

}

bool dynamic_context::contains_module(const u32 id) const
{
    // NOT YET IMPLEMENTED
    return false;
}

bool dynamic_context::contains_gate(const u32 id) const
{
    // NOT YET IMPLEMENTED
    return false;
}

bool dynamic_context::contains_net(const u32 id) const
{
    // NOT YET IMPLEMENTED
    return false;
}

QString dynamic_context::name() const
{
    return m_name;
}

void dynamic_context::apply_changes()
{
    assert(m_unapplied_changes);
    // APPLY STUFF
    m_unapplied_changes = false;
    schedule_relayout();
}

const QSet<u32>& dynamic_context::gates() const
{
    return m_gates;
}

const QSet<u32>& dynamic_context::modules() const
{
    return m_modules;
}

const QSet<u32>& dynamic_context::nets() const
{
    return m_nets;
}
