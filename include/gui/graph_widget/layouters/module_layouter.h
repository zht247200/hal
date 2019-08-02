#ifndef MODULE_LAYOUTER_H
#define MODULE_LAYOUTER_H

#include "gui/graph_widget/layouters/graph_layouter.h"

class module_context;

class module_layouter : public graph_layouter
{
public:
    explicit module_layouter(const module_context* const context);

    virtual void add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& internal_nets, const QSet<u32>& global_io_nets, const QSet<u32>& local_io_nets)    = 0;
    virtual void remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& internal_nets, const QSet<u32>& global_io_nets, const QSet<u32>& local_io_nets) = 0;

protected:
    const module_context* const m_context;
};

#endif // MODULE_LAYOUTER_H
