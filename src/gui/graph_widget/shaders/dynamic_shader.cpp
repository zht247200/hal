#include "gui/graph_widget/shaders/dynamic_shader.h"

dynamic_shader::dynamic_shader(const dynamic_context* const context) :
    m_context(context)
{

}

void dynamic_shader::add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}

void dynamic_shader::remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(nets)
}
