#ifndef STANDARD_MODULE_SHADER_H
#define STANDARD_MODULE_SHADER_H

#include "gui/graph_widget/shaders/module_shader.h"

class standard_module_shader final : public module_shader
{
public:
    standard_module_shader(const module_context* const context);

    void update() override;

private:
    static bool s_color_gates;
};

#endif // STANDARD_MODULE_SHADER_H
