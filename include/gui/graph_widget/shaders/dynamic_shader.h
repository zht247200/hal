#ifndef DYNAMIC_SHADER_H
#define DYNAMIC_SHADER_H

#include "gui/graph_widget/shaders/graph_shader.h"

class dynamic_context;

class dynamic_shader : public graph_shader
{
public:
    dynamic_shader(const dynamic_context* const context);

    virtual void add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);
    virtual void remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);

protected:
    const dynamic_context* const m_context;
};

#endif // DYNAMIC_SHADER_H
