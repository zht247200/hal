#ifndef DYNAMIC_LAYOUTER_H
#define DYNAMIC_LAYOUTER_H

#include "gui/graph_widget/layouters/graph_layouter.h"

class dynamic_context;

class dynamic_layouter : public graph_layouter
{
public:
    explicit dynamic_layouter(const dynamic_context* const context);

    virtual void add(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets)    = 0;
    virtual void remove(const QSet<u32> modules, const QSet<u32> gates, const QSet<u32> nets) = 0;
};

#endif // DYNAMIC_LAYOUTER_H
