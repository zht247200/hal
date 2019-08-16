#ifndef GRAPH_SHADER_H
#define GRAPH_SHADER_H

#include "def.h"

#include "gui/graph_widget/items/graphics_net.h"
#include "gui/graph_widget/items/graphics_node.h"

#include <QColor>
#include <QMap>
#include <QSet>
#include <QVector>

class graph_context;

class graph_shader
{
public:
    struct shading
    {
        QMap<u32, graphics_node::visuals> module_visuals;
        QMap<u32, graphics_node::visuals> gate_visuals;
        QMap<u32, graphics_net::visuals> net_visuals;
    };

    graph_shader();
    virtual ~graph_shader() = default;

    virtual void update() = 0;

    const shading& get_shading();

protected:
    shading m_shading;
};

#endif // GRAPH_SHADER_H
