#ifndef STANDARD_DYNAMIC_SHADER_H
#define STANDARD_DYNAMIC_SHADER_H

#include "gui/graph_widget/shaders/dynamic_shader.h"

class standard_dynamic_shader final : public dynamic_shader
{
public:
    standard_dynamic_shader(const dynamic_context* const context);

    void update() override;

private:
    static bool s_color_gates;
};

#endif // STANDARD_DYNAMIC_SHADER_H
