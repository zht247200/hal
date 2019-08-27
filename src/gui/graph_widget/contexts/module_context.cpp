#include "gui/graph_widget/contexts/module_context.h"

#include "gui/graph_widget/layouters/minimal_module_layouter.h"
#include "gui/graph_widget/layouters/standard_module_layouter.h"
#include "gui/graph_widget/shaders/standard_module_shader.h"
#include "gui/gui_globals.h"

module_context::layouter module_context::s_default_layouter = module_context::layouter::standard;
module_context::shader module_context::s_default_shader = module_context::shader::standard;

module_context::module_context(const std::shared_ptr<const module> m) : graph_context(type::module, create_layouter(s_default_layouter, this), create_shader(s_default_shader, this)),
    m_id(m->get_id()),
    m_layouter_type(s_default_layouter)
{
    for (const std::shared_ptr<module>& s : m->get_submodules())
        m_modules.insert(s->get_id());

    for (const std::shared_ptr<gate>& g : m->get_gates())
        m_gates.insert(g->get_id());

    for (const std::shared_ptr<net>& n : m->get_input_nets())
    {
        if (n->is_unrouted())
            m_global_io_nets.insert(n->get_id());
        else
            m_local_io_nets.insert(n->get_id());
    }

    for (const std::shared_ptr<net>& n : m->get_output_nets())
    {
        if (n->is_unrouted())
            m_global_io_nets.insert(n->get_id());
        else
            m_local_io_nets.insert(n->get_id());
    }

    for (const std::shared_ptr<net>& n: m->get_internal_nets())
    {
        if (!m_local_io_nets.contains(n->get_id()))
            m_internal_nets.insert(n->get_id());
    }

    static_cast<module_layouter*>(m_layouter)->add(m_modules, m_gates, m_internal_nets, m_local_io_nets, m_global_io_nets);
    static_cast<module_shader*>(m_shader)->add(m_modules, m_gates, m_internal_nets);

    schedule_relayout();
}

bool module_context::contains_module(const u32 id) const
{
    return (m_modules - m_removed_modules + m_added_modules).contains(id);
}

bool module_context::contains_gate(const u32 id) const
{
    return (m_gates - m_removed_gates + m_added_gates).contains(id);
}

bool module_context::contains_net(const u32 id) const
{
    return (m_internal_nets - m_removed_internal_nets + m_added_internal_nets +
            m_local_io_nets - m_removed_local_io_nets + m_added_local_io_nets +
            m_global_io_nets - m_removed_global_io_nets + m_added_global_io_nets).contains(id);
}

void module_context::add(const QSet<u32>& modules, const QSet<u32>& gates)
{
    // ASSERT INPUT VALIDITY ???

    QSet<u32> new_modules = modules - m_modules;
    QSet<u32> new_gates = gates - m_gates;

    QSet<u32> old_modules = m_removed_modules & new_modules;
    QSet<u32> old_gates = m_removed_gates & new_gates;

    m_removed_modules -= old_modules;
    m_removed_gates -= old_gates;

    new_modules -= old_modules;
    new_gates -= old_gates;

    m_added_modules += new_modules;
    m_added_gates += new_gates;

    // NET STUFF, CAN PROBABLY BE OPTIMIZED
    std::shared_ptr<module> m = g_netlist->get_module_by_id(m_id);
    assert(m);

    QSet<u32> current_internal_nets;
    for (const std::shared_ptr<net>& n: m->get_internal_nets())
        current_internal_nets.insert(n->get_id());

    m_added_internal_nets = current_internal_nets - m_internal_nets;
    m_removed_internal_nets -= current_internal_nets;

    QSet<u32> current_global_io_nets;
    QSet<u32> current_local_io_nets;

    for (const std::shared_ptr<net>& n : m->get_input_nets())
    {
        if (n->is_unrouted())
            current_global_io_nets.insert(n->get_id());
        else
            current_local_io_nets.insert(n->get_id());
    }

    for (const std::shared_ptr<net>& n : m->get_output_nets())
    {
        if (n->is_unrouted())
            current_global_io_nets.insert(n->get_id());
        else
            current_local_io_nets.insert(n->get_id());
    }

    m_added_local_io_nets = current_local_io_nets - m_local_io_nets;
    m_removed_local_io_nets -= current_local_io_nets;

    m_added_global_io_nets = current_global_io_nets - m_global_io_nets;
    m_removed_global_io_nets -= current_global_io_nets;

    evaluate_changes();
    update();
}

void module_context::remove(const QSet<u32>& modules, const QSet<u32>& gates)
{
    // ASSERT INPUT VALIDITY ???

    QSet<u32> old_modules = modules & m_modules;
    QSet<u32> old_gates = gates & m_gates;

    m_removed_modules += old_modules;
    m_removed_gates += old_gates;

    m_added_modules -= modules;
    m_added_gates -= gates;

    // NET STUFF, CAN PROBABLY BE OPTIMIZED
    std::shared_ptr<module> m = g_netlist->get_module_by_id(m_id);
    assert(m);

    QSet<u32> current_internal_nets;
    for (const std::shared_ptr<net>& n: m->get_internal_nets())
        current_internal_nets.insert(n->get_id());

    m_added_internal_nets = current_internal_nets - m_internal_nets;
    m_removed_internal_nets = m_internal_nets - current_internal_nets;

    QSet<u32> current_global_io_nets;
    QSet<u32> current_local_io_nets;

    for (const std::shared_ptr<net>& n : m->get_input_nets())
    {
        if (n->is_unrouted())
            current_global_io_nets.insert(n->get_id());
        else
            current_local_io_nets.insert(n->get_id());
    }

    for (const std::shared_ptr<net>& n : m->get_output_nets())
    {
        if (n->is_unrouted())
            current_global_io_nets.insert(n->get_id());
        else
            current_local_io_nets.insert(n->get_id());
    }

    m_added_local_io_nets = current_local_io_nets - m_local_io_nets;
    m_removed_local_io_nets = m_local_io_nets - current_local_io_nets;

    m_added_global_io_nets = current_global_io_nets - m_global_io_nets;
    m_removed_global_io_nets = m_global_io_nets - current_global_io_nets;

    evaluate_changes();
    update();
}

bool module_context::node_for_gate(hal::node& node, const u32 id) const
{
    // MIGHT BE BETTER TO CHECK FOR A SPECIFIC IO PIN
    if (m_gates.contains(id))
    {
        node.id = id;
        node.type = hal::node_type::gate;
        return true;
    }

    std::shared_ptr<gate> g = g_netlist->get_gate_by_id(id);

    if (!g)
        return false;

    std::shared_ptr<module> m = g->get_module();

    while (m)
    {
        if (m_modules.contains(m->get_id()))
        {
            node.id = m->get_id();
            node.type = hal::node_type::module;
            return true;
        }

        m = m->get_parent_module();
    }

    return false;
}

u32 module_context::get_id() const
{
    return m_id;
}

const QSet<u32>& module_context::modules() const
{
    return m_modules;
}

const QSet<u32>& module_context::gates() const
{
    return m_gates;
}

const QSet<u32>& module_context::internal_nets() const
{
    return m_internal_nets;
}

const QSet<u32>& module_context::local_io_nets() const
{
    return m_local_io_nets;
}

const QSet<u32>& module_context::global_io_nets() const
{
    return m_global_io_nets;
}

module_layouter* module_context::create_layouter(const module_context::layouter type, module_context* const context)
{
    switch (type)
    {
    case layouter::standard: return new standard_module_layouter(context);
    case layouter::minimal: return new minimal_module_layouter(context);
    }
}

module_shader* module_context::create_shader(const module_context::shader type, module_context* const context)
{
    switch (type)
    {
    case shader::standard: return new standard_module_shader(context);
    }
}

void module_context::apply_changes()
{
    assert(m_unapplied_changes);

    m_modules -= m_removed_modules;
    m_gates -= m_removed_gates;
    m_internal_nets -= m_removed_internal_nets;
    m_local_io_nets -= m_removed_local_io_nets;
    m_global_io_nets -= m_removed_global_io_nets;

    m_modules += m_added_modules;
    m_gates += m_added_gates;
    m_internal_nets += m_added_internal_nets;
    m_local_io_nets += m_added_local_io_nets;
    m_global_io_nets += m_added_global_io_nets;

    static_cast<module_layouter*>(m_layouter)->remove(m_removed_modules, m_removed_gates, m_removed_internal_nets, m_removed_local_io_nets, m_removed_global_io_nets);
    static_cast<module_layouter*>(m_layouter)->add(m_added_modules, m_added_gates, m_added_internal_nets, m_added_local_io_nets, m_added_global_io_nets);

    static_cast<module_shader*>(m_shader)->remove(m_removed_modules, m_removed_gates, m_removed_internal_nets);
    static_cast<module_shader*>(m_shader)->add(m_added_modules, m_added_gates, m_added_internal_nets);

    m_added_modules.clear();
    m_added_gates.clear();
    m_added_internal_nets.clear();
    m_added_local_io_nets.clear();
    m_added_global_io_nets.clear();

    m_removed_modules.clear();
    m_removed_gates.clear();
    m_removed_internal_nets.clear();
    m_removed_local_io_nets.clear();
    m_removed_global_io_nets.clear();

    m_unapplied_changes = false;

    schedule_relayout();
}

void module_context::evaluate_changes()
{
    if (!m_added_modules.isEmpty()         ||
        !m_added_gates.isEmpty()           ||
        !m_added_internal_nets.isEmpty()   ||
        !m_added_local_io_nets.isEmpty()   ||
        !m_added_global_io_nets.isEmpty()  ||
        !m_removed_modules.isEmpty()       ||
        !m_removed_gates.isEmpty()         ||
        !m_removed_internal_nets.isEmpty() ||
        !m_removed_local_io_nets.isEmpty() ||
        !m_removed_global_io_nets.isEmpty())

        m_unapplied_changes = true;
}
