#ifndef GRAPH_CONTEXT_H
#define GRAPH_CONTEXT_H

#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/graph_widget/shaders/graph_shader.h"
#include "gui/gui_def.h"

#include <QObject>
#include <QSet>

class graph_context_subscriber;

class graph_context : public QObject
{
    Q_OBJECT

public:
    enum class type
    {
        module,
        cone,
        dynamic
    };

    explicit graph_context(type t, graph_layouter* layouter, graph_shader* shader, QObject* parent = nullptr);
    virtual ~graph_context();

    void subscribe(graph_context_subscriber* const subscriber);
    void unsubscribe(graph_context_subscriber* const subscriber);

    void schedule_relayout();
    void schedule_reshade();

    type get_type();
    graphics_scene* scene();

    bool scene_available() const;

protected:
    void update();

    graph_layouter* m_layouter;
    graph_shader* m_shader; // MOVE SHADER TO VIEW ? USE BASE SHADER AND ADDITIONAL SHADERS ? LAYER SHADERS ?

    bool m_unapplied_changes;

private Q_SLOTS:
    void handle_layouter_update(const int percent);
    void handle_layouter_update(const QString& message);
    void handle_layouter_task_finished();

private:
    virtual void apply_changes() = 0;

    void queue_layouter_task();
    void reshade();

    const type m_type;

    QList<graph_context_subscriber*> m_subscribers;

    bool m_scene_available;

    bool m_relayout_required;
    bool m_reshade_required;
};

#endif // GRAPH_CONTEXT_H
