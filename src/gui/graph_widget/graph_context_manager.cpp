#include "gui/graph_widget/graph_context_manager.h"

#include "gui/graph_widget/contexts/cone_context.h"
#include "gui/graph_widget/contexts/dynamic_context.h"
#include "gui/graph_widget/contexts/module_context.h"
#include "gui/graph_widget/layouters/debug_layouter.h"
#include "gui/graph_widget/layouters/standard_cone_layouter.h"
#include "gui/graph_widget/layouters/standard_module_layouter.h"
#include "gui/graph_widget/layouters/standard_module_layouter_v2.h"
#include "gui/graph_widget/layouters/minimal_module_layouter.h"
#include "gui/graph_widget/shaders/standard_dynamic_shader.h"
#include "gui/graph_widget/shaders/standard_module_shader.h"
#include "gui/gui_globals.h"

int graph_context_manager::s_max_module_contexts = 10; // USE SETTINGS FOR THIS

graph_context_manager::graph_context_manager()
{

}

void graph_context_manager::set_max_module_contexts(const int max)
{
    if (max > 0)
        s_max_module_contexts = max;
}

module_context* graph_context_manager::get_module_context(const u32 id)
{
    // METHOD EXPECTS VALID ID

    for (int i = 0; i < m_module_contexts.size(); ++i)
        if (m_module_contexts.at(i)->get_id() == id)
        {
            if (i > 0)
                m_module_contexts.move(i, 0);

            return m_module_contexts[0];
        }

    std::shared_ptr<module> m = g_netlist->get_module_by_id(id);
    assert(m);

    assert(s_max_module_contexts > 0);

    if (m_module_contexts.size() == s_max_module_contexts)
        m_module_contexts.removeLast();

    module_context* c = new module_context(m);
    m_module_contexts.prepend(c);
    return c;
}

cone_context* graph_context_manager::add_cone_context(const QString& name)
{
    cone_context* context = new cone_context(name);
    m_cone_contexts.append(context);
    return context;
}

void graph_context_manager::delete_cone_context(const QString& name)
{
    Q_UNUSED(name)
}

cone_context* graph_context_manager::get_cone_context(const QString& name)
{
    for (cone_context* context : m_cone_contexts)
        if (context->name() == name)
            return context;

    return nullptr;
}

QStringList graph_context_manager::cone_context_list() const
{
    QStringList list;

    for (cone_context* context : m_cone_contexts)
        list.append(context->name());

    return list;
}

dynamic_context* graph_context_manager::add_dynamic_context(const QString& name)
{
    dynamic_context* context = new dynamic_context(name);
    m_dynamic_contexts.append(context);
    return context;
}

void graph_context_manager::delete_dynamic_context(const QString& name)
{
    Q_UNUSED(name)
}

dynamic_context* graph_context_manager::get_dynamic_context(const QString& name)
{
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->name() == name)
            return context;

    return nullptr;
}

QStringList graph_context_manager::dynamic_context_list() const
{
    QStringList list;

    for (dynamic_context* context : m_dynamic_contexts)
        list.append(context->name());

    return list;
}

void graph_context_manager::handle_module_removed(const std::shared_ptr<module> m)
{
    // REMOVE MODULE CONTEXT
    for (int i = 0; i < m_module_contexts.size(); ++i)
        if (m_module_contexts[i]->get_id() == m->get_id())
        {
            delete m_module_contexts[i];
            m_module_contexts.remove(i);
            break;
        }

    // REMOVE MODULE FROM DYNAMIC CONTEXTS
//    for (dynamic_context* context : m_dynamic_contexts)
//        if (context->modules().contains(m->get_id()))
//            context->remove(QSet<u32>{m->get_id()}, QSet<u32>(), QSet<u32>());

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_name_changed(const std::shared_ptr<module> m) const
{
    // UPDATE MODULE CONTEXTS
    for (module_context* context : m_module_contexts)
        if (context->modules().contains(m->get_id()))
            context->schedule_relayout();

    // UPDATE DYNAMIC CONTEXTS
    for (dynamic_context* context : m_dynamic_contexts)
        if (context->modules().contains(m->get_id()))
            context->schedule_relayout();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_submodule_added(const std::shared_ptr<module> m, const u32 added_module) const
{
    for (module_context* context : m_module_contexts)
        if (context->get_id() == m->get_id())
        {
            context->add(QSet<u32>{added_module}, QSet<u32>());
            break;
        }

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_submodule_removed(const std::shared_ptr<module> m, const u32 removed_module) const
{
    for (module_context* context : m_module_contexts)
        if (context->get_id() == m->get_id())
        {
            context->remove(QSet<u32>{removed_module}, QSet<u32>());
            break;
        }

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_gate_assigned(const std::shared_ptr<module> m, const u32 inserted_gate) const
{
    for (module_context* context : m_module_contexts)
        if (context->get_id() == m->get_id())
        {
            context->add(QSet<u32>(), QSet<u32>{inserted_gate});
            break;
        }

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_module_gate_removed(const std::shared_ptr<module> m, const u32 removed_gate) const
{
    for (module_context* context : m_module_contexts)
        if (context->get_id() == m->get_id())
        {
            context->remove(QSet<u32>(), QSet<u32>{removed_gate});
            break;
        }

    // TRIGGER RELAYOUT FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_gate_name_changed(const std::shared_ptr<gate> g) const
{
    // IF GATE IN CONTEXT REQUEST UPDATE
    for (module_context* context : m_module_contexts)
        if (context->gates().contains(g->get_id()))
            context->schedule_relayout();

    for (dynamic_context* context : m_dynamic_contexts)
        if (context->gates().contains(g->get_id()))
            context->schedule_relayout();

    // TRIGGER RESHADE FOR ALL CONTEXTS THAT RECURSIVELY CONTAIN THE MODULE
}

void graph_context_manager::handle_net_created(const std::shared_ptr<net> n) const
{
    Q_UNUSED(n)

    // CAN NETS BE CREATED WITH SRC AND DST INFORMATION ?
    // IF NO THIS EVENT DOESNT NEED TO BE HANDLED
}

void graph_context_manager::handle_net_removed(const std::shared_ptr<net> n) const
{
    for (module_context* const context : m_module_contexts)
        if (context->internal_nets().contains(n->get_id()) || context->local_io_nets().contains(n->get_id()) || context->global_io_nets().contains(n->get_id()))
            context->schedule_relayout();
    // CANT REMOVE NET FROM MODULE CONTEXT ATM, FIX

    for (dynamic_context* const context : m_dynamic_contexts)
        if (context->nets().contains(n->get_id()))
            context->remove(QSet<u32>(), QSet<u32>(), QSet<u32>{n->get_id()});
        else
            context->schedule_relayout();
}

void graph_context_manager::handle_net_name_changed(const std::shared_ptr<net> n) const
{
    Q_UNUSED(n)

    debug_relayout_all();
}

void graph_context_manager::handle_net_src_changed(const std::shared_ptr<net> n) const
{
    Q_UNUSED(n)

    debug_relayout_all();

//    for (module_context* context : m_module_contexts)
//        if (context->nets().contains(n->get_id()))
//            context->request_update();

//    for (dynamic_context* context : m_dynamic_contexts)
//        if (context->nets().contains(n->get_id()))
//            context->request_update();
}

void graph_context_manager::handle_net_dst_added(const std::shared_ptr<net> n, const u32 dst_gate_id) const
{
    Q_UNUSED(n)
    Q_UNUSED(dst_gate_id)

    debug_relayout_all();

//    for (module_context* const context : m_module_contexts)
//        if (context->internal_nets().contains(n->get_id()) || context->local_io_nets().contains(n->get_id()) || context->global_io_nets().contains(n->get_id()) || context->gates().contains(dst_gate_id))
//            context->schedule_relayout();

//    for (dynamic_context* const context : m_dynamic_contexts)
//        if (context->nets().contains(n->get_id()) || context->gates().contains(dst_gate_id))
//            context->schedule_relayout();
}

void graph_context_manager::handle_net_dst_removed(const std::shared_ptr<net> n, const u32 dst_gate_id) const
{
    Q_UNUSED(n)
    Q_UNUSED(dst_gate_id)

    debug_relayout_all();

//    for (module_context* context : m_module_contexts)
//        if (context->internal_nets().contains(n->get_id()) || context->local_io_nets().contains(n->get_id()) || context->global_io_nets().contains(n->get_id()))
//            context->schedule_relayout();

//    for (dynamic_context* context : m_dynamic_contexts)
//        if (context->nets().contains(n->get_id()))
//            context->schedule_relayout();
}

cone_layouter *graph_context_manager::get_default_layouter(cone_context* const context) const
{
    // USE SETTINGS + FACTORY
    return new standard_cone_layouter(context);
}

cone_shader *graph_context_manager::get_default_shader(cone_context* const context) const
{
    // USE SETTINGS + FACTORY
    //return new standard_dynamic_shader(context);
    return nullptr;
}

void graph_context_manager::debug_relayout_all() const
{
    for (module_context* const context : m_module_contexts)
            context->schedule_relayout();

    for (cone_context* const context : m_cone_contexts)
            context->schedule_relayout();

    for (dynamic_context* const context : m_dynamic_contexts)
            context->schedule_relayout();
}
