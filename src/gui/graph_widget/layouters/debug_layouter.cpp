#include "gui/graph_widget/layouters/debug_layouter.h"

#include <QSet>
#include <QTime>

debug_layouter::debug_layouter(const module_context* const context) : module_layouter(context)
{

}

QString debug_layouter::name() const
{
    return "Debug Layouter";
}

QString debug_layouter::description() const
{
    return "For testing purposes";
}

void debug_layouter::layout()
{
    // ARTIFICIAL LAG FOR TESTING PURPOSES
    QTime time = QTime::currentTime().addSecs(10);
    while (QTime::currentTime() < time)
    {
        Q_EMIT status_update("I'm doing stuff (~￣▽￣)~");
    }
}

void debug_layouter::add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& internal_nets, const QSet<u32>& local_io_nets, const QSet<u32>& global_io_nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(internal_nets)
    Q_UNUSED(local_io_nets)
    Q_UNUSED(global_io_nets)
}

void debug_layouter::remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& internal_nets, const QSet<u32>& local_io_nets, const QSet<u32>& global_io_nets)
{
    Q_UNUSED(modules)
    Q_UNUSED(gates)
    Q_UNUSED(internal_nets)
    Q_UNUSED(local_io_nets)
    Q_UNUSED(global_io_nets)
}
