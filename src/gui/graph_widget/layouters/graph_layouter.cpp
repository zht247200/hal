#include "gui/graph_widget/layouters/graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_scene.h"

graph_layouter::graph_layouter(QObject* parent) : QObject(parent),
    m_scene(new graphics_scene(this))
{

}

graphics_scene* graph_layouter::scene() const
{
    return m_scene;
}
