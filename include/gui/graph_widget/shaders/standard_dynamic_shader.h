#ifndef STANDARD_DYNAMIC_SHADER_H
#define STANDARD_DYNAMIC_SHADER_H

#include "gui/graph_widget/shaders/graph_shader.h"

class standard_dynamic_shader final : public graph_shader
{
public:
    standard_dynamic_shader(const graph_context* const context);

    void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;
    void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) override;

    void update() override;

private:
    static bool s_color_gates;
};

#endif // STANDARD_DYNAMIC_SHADER_H
