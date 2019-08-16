#ifndef MODULE_SHADER_H
#define MODULE_SHADER_H

#include "gui/graph_widget/shaders/graph_shader.h"

class module_context;

class module_shader : public graph_shader
{
public:
    module_shader(const module_context* const context);

    virtual void add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);
    virtual void remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);

protected:
    const module_context* const m_context;
};

#endif // MODULE_SHADER_H
