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
    enum class context_type
    {
        module,
        cone,
        dynamic
    };

    explicit graph_context(context_type type, graph_layouter* layouter, graph_shader* shader, QObject* parent = nullptr);
    virtual ~graph_context();

    void subscribe(graph_context_subscriber* const subscriber);
    void unsubscribe(graph_context_subscriber* const subscriber);

    graphics_scene* scene();

    context_type get_type();

    bool available() const;
    bool update_in_progress() const;

    void update();

    void request_update();

    bool node_for_gate(hal::node& node, const u32 id) const;

private Q_SLOTS:
    void handle_layouter_update(const int percent);
    void handle_layouter_update(const QString& message);
    void handle_layouter_finished();

protected:
    void evaluate_changes();

    bool m_unhandled_changes;
    bool m_scene_update_required;

    bool m_update_requested;

private:
    void apply_changes();
    void update_scene();

    const context_type m_type;

    graph_layouter* m_layouter;
    graph_shader* m_shader; // MOVE SHADER TO VIEW ? USE BASE SHADER AND ADDITIONAL SHADERS ? LAYER SHADERS ?

    QList<graph_context_subscriber*> m_subscribers;

    bool m_scene_available;
    bool m_update_in_progress;
};

#endif // GRAPH_CONTEXT_H
