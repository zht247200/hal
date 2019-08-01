#include "gui/graph_widget/contexts/graph_context.h"

#include "netlist/module.h"

#include "gui/graph_widget/contexts/graph_context_subscriber.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/layouters/layouter_task.h"
#include "gui/gui_globals.h"

static const bool lazy_updates = false; // USE SETTINGS FOR THIS

graph_context::graph_context(context_type type, graph_layouter* layouter, graph_shader* shader, QObject* parent) : QObject(parent),
    m_unhandled_changes(false),
    m_scene_update_required(false),
    m_update_requested(false),
    m_type(type),
    m_layouter(layouter),
    m_shader(shader),
    m_scene_available(true),
    m_update_in_progress(false)
{
    connect(m_layouter, qOverload<int>(&graph_layouter::status_update), this, qOverload<int>(&graph_context::handle_layouter_update), Qt::ConnectionType::QueuedConnection);
    connect(m_layouter, qOverload<const QString&>(&graph_layouter::status_update), this, qOverload<const QString&>(&graph_context::handle_layouter_update), Qt::ConnectionType::QueuedConnection);
}

graph_context::~graph_context()
{
    for (graph_context_subscriber* subscriber : m_subscribers)
        subscriber->handle_context_about_to_be_deleted();

    delete m_layouter;
    delete m_shader;
}

void graph_context::subscribe(graph_context_subscriber* const subscriber)
{
    assert(subscriber);
    assert(!m_subscribers.contains(subscriber));

    m_subscribers.append(subscriber);
    update();
}

void graph_context::unsubscribe(graph_context_subscriber* const subscriber)
{
    assert(m_subscribers.contains(subscriber));

    m_subscribers.removeOne(subscriber);
}

graphics_scene* graph_context::scene()
{
    return m_layouter->scene();
}

graph_context::context_type graph_context::get_type()
{
    return m_type;
}

bool graph_context::available() const
{
    return m_scene_available;
}

bool graph_context::update_in_progress() const
{
    return m_update_in_progress;
}

void graph_context::handle_layouter_finished()
{
    if (m_unhandled_changes)
        apply_changes();

    if (m_scene_update_required)
        update_scene();
    else
    {
        // SHADER MIGHT HAS TO BE THREADED ASWELL, DEPENDING ON COMPLEXITY
        m_shader->update();
        m_layouter->scene()->update_visuals(m_shader->get_shading());

        m_update_in_progress = false;

        m_layouter->scene()->connect_all();

        for (graph_context_subscriber* s : m_subscribers)
            s->handle_scene_available();
    }
}

void graph_context::update()
{
    if (m_update_in_progress)
        return;

    if (lazy_updates)
        if (m_subscribers.empty())
            return;

    if (m_unhandled_changes)
        apply_changes();

    if (m_scene_update_required)
        update_scene();
}

void graph_context::request_update()
{
    m_update_requested = true;

    if (m_update_in_progress)
        return;

    if (lazy_updates)
        if (m_subscribers.empty())
            return;

    //update();
}

bool graph_context::node_for_gate(hal::node& node, const u32 id) const
{
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

void graph_context::handle_layouter_update(const int percent)
{
    for (graph_context_subscriber* s : m_subscribers)
        s->handle_status_update(percent);
}

void graph_context::handle_layouter_update(const QString& message)
{
    for (graph_context_subscriber* s : m_subscribers)
        s->handle_status_update(message);
}

void graph_context::update_scene()
{
    for (graph_context_subscriber* s : m_subscribers)
        s->handle_scene_unavailable();

    m_layouter->scene()->disconnect_all();

    m_update_in_progress = true;
    m_scene_update_required = false;

    layouter_task* task = new layouter_task(m_layouter);
    connect(task, &layouter_task::finished, this, &graph_context::handle_layouter_finished, Qt::ConnectionType::QueuedConnection);
    g_thread_pool->queue_task(task);
}
