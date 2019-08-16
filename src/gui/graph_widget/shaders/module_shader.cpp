#include "gui/graph_widget/shaders/module_shader.h"

#include "gui/graph_widget/contexts/module_context.h"

module_shader::module_shader(const module_context* const context) :
    m_context(context)
{

}

void module_shader::add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void module_shader::remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}
