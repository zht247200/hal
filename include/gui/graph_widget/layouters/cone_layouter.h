#ifndef CONE_LAYOUTER_H
#define CONE_LAYOUTER_H

#include "gui/graph_widget/layouters/graph_layouter.h"

class cone_context;

class cone_layouter : public graph_layouter
{
public:
    explicit cone_layouter(const cone_context* const context);

    virtual void expand(const u32 from_gate, const u32 via_net, const u32 to_gate) = 0;
    // HOW SHOULD THE LAYOUTER DEAL WITH REMOVED GATES / NETS ???
};

#endif // CONE_LAYOUTER_H
