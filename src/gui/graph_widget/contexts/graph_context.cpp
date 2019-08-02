#include "gui/graph_widget/contexts/graph_context.h"

#include "netlist/module.h"

#include "gui/graph_widget/contexts/graph_context_subscriber.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/layouters/layouter_task.h"
#include "gui/gui_globals.h"

static const bool lazy_updates = false; // USE SETTINGS FOR THIS

graph_context::graph_context(type t, graph_layouter* layouter, graph_shader* shader, QObject* parent) : QObject(parent),
    m_layouter(layouter),
    m_shader(shader),
    m_unapplied_changes(false),
    m_type(t),
    m_scene_available(true),
    m_relayout_required(false),
    m_reshade_required(false)
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

void graph_context::schedule_relayout()
{
    m_relayout_required = true;
    update();
}

void graph_context::schedule_reshade()
{
    m_reshade_required = true;
    update();
}

graphics_scene* graph_context::scene()
{
    return m_layouter->scene();
}

graph_context::type graph_context::get_type()
{
    return m_type;
}

bool graph_context::scene_available() const
{
    return m_scene_available;
}

void graph_context::update()
{
    if (!m_scene_available)
        return;

    if (lazy_updates)
        if (m_subscribers.empty())
            return;

    if (m_unapplied_changes)
        apply_changes();

    if (m_relayout_required)
        queue_layouter_task();
    else if (m_reshade_required)
        reshade();
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

void graph_context::handle_layouter_task_finished()
{
    if (m_unapplied_changes)
        apply_changes();
    else
    {
        reshade();

        m_layouter->scene()->connect_all();
        m_scene_available = true;

        for (graph_context_subscriber* s : m_subscribers)
            s->handle_scene_available();
    }
}

void graph_context::queue_layouter_task()
{
    m_scene_available = false;

    for (graph_context_subscriber* s : m_subscribers)
        s->handle_scene_unavailable();

    m_layouter->scene()->disconnect_all();

    m_relayout_required = false;
    m_reshade_required = true;

    layouter_task* task = new layouter_task(m_layouter);
    connect(task, &layouter_task::finished, this, &graph_context::handle_layouter_task_finished, Qt::ConnectionType::QueuedConnection);
    g_thread_pool->queue_task(task);
}

void graph_context::reshade()
{
    // SHADER MIGHT HAS TO BE THREADED ASWELL, DEPENDING ON COMPLEXITY
    m_shader->update();
    m_layouter->scene()->update_visuals(m_shader->get_shading());
    m_reshade_required = false;
}
