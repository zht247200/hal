#ifndef CONE_SHADER_H
#define CONE_SHADER_H

#include "gui/graph_widget/shaders/graph_shader.h"

class cone_context;

class cone_shader : public graph_shader
{
public:
    cone_shader(const cone_context* const context);

protected:
    const cone_context* const m_context;
};

#endif // CONE_SHADER_H
