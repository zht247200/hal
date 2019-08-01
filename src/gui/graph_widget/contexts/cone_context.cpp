#include "gui/graph_widget/contexts/cone_context.h"

#include "gui/gui_globals.h"

cone_context::cone_context(const QString& name) : graph_context(context_type::dynamic, g_graph_context_manager.get_default_layouter(this), g_graph_context_manager.get_default_shader(this)),
    m_name(name)
{

}

QString cone_context::name() const
{
    return m_name;
}
