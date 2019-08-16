#include "gui/graph_widget/contexts/dynamic_context.h"

#include "gui/graph_widget/layouters/dynamic_layouter.h"
#include "gui/gui_globals.h"

bool dynamic_context::m_expand_all_nets = false;
bool dynamic_context::m_expand_gnd_nets = false;
bool dynamic_context::m_expand_vcc_nets = false;
bool dynamic_context::m_expand_global_nets = false;

dynamic_context::dynamic_context(const QString& name) : graph_context(type::dynamic, g_graph_context_manager.get_default_layouter(this), g_graph_context_manager.get_default_shader(this)),
    m_name(name)
{

}

bool dynamic_context::contains_module(const u32 id) const
{
    return m_modules.contains(id);
}

bool dynamic_context::contains_gate(const u32 id) const
{
    return m_gates.contains(id);
}

bool dynamic_context::contains_net(const u32 id) const
{
    return m_nets.contains(id);
}

void dynamic_context::add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
{
    // ASSERT INPUT VALIDITY ???

    QSet<u32> new_modules = modules - m_modules;
    QSet<u32> new_gates = gates - m_gates;
    QSet<u32> new_nets = nets - m_nets;

    QSet<u32> old_modules = m_removed_modules & new_modules;
    QSet<u32> old_gates = m_removed_gates & new_gates;
    QSet<u32> old_nets = m_removed_nets & new_nets;

    m_removed_modules -= old_modules;
    m_removed_gates -= old_gates;
    m_removed_gates -= old_nets;

    new_modules -= old_modules;
    new_gates -= old_gates;
    new_nets -= old_nets;

    m_added_modules += new_modules;
    m_added_gates += new_gates;
    m_added_nets += new_nets;

    // ADD NETS IF NECESSARY

    evaluate_changes();
    update();
}

void dynamic_context::remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
{
    // ASSERT INPUT VALIDITY ???

    QSet<u32> old_modules = modules & m_modules;
    QSet<u32> old_gates = gates & m_gates;
    QSet<u32> old_nets = nets & m_nets;

    m_removed_modules += old_modules;
    m_removed_gates += old_gates;
    m_removed_nets += old_nets;

    m_added_modules -= modules;
    m_added_gates -= gates;
    m_added_nets -= nets;

    // ADD NETS IF NECESSARY

    evaluate_changes();
    update();
}

const QSet<u32>& dynamic_context::modules() const
{
    return m_modules;
}

const QSet<u32>& dynamic_context::gates() const
{
    return m_gates;
}

const QSet<u32>& dynamic_context::nets() const
{
    return m_nets;
}

QString dynamic_context::name() const
{
    return m_name;
}

void dynamic_context::apply_changes()
{
    assert(m_unapplied_changes);

    m_modules -= m_removed_modules;
    m_gates -= m_removed_gates;
    m_nets -= m_removed_nets;

    m_modules += m_added_modules;
    m_gates += m_added_gates;
    m_nets += m_added_nets;

    static_cast<dynamic_layouter*>(m_layouter)->remove(m_removed_modules, m_removed_gates, m_removed_nets);
    static_cast<dynamic_layouter*>(m_layouter)->add(m_added_modules, m_added_gates, m_added_nets);

    m_shader->remove(m_removed_modules, m_removed_gates, m_removed_nets);
    m_shader->add(m_added_modules, m_added_gates, m_added_nets);

    m_added_modules.clear();
    m_added_gates.clear();
    m_added_nets.clear();

    m_removed_modules.clear();
    m_removed_gates.clear();
    m_removed_nets.clear();

    m_unapplied_changes = false;

    schedule_relayout();
}

void dynamic_context::evaluate_changes()
{
    if (!m_added_modules.isEmpty()   ||
        !m_added_gates.isEmpty()     ||
        !m_added_nets.isEmpty()      ||
        !m_removed_modules.isEmpty() ||
        !m_removed_gates.isEmpty()   ||
        !m_removed_nets.isEmpty())

        m_unapplied_changes = true;
}
