#ifndef DEBUG_LAYOUTER_H
#define DEBUG_LAYOUTER_H

#include "gui/graph_widget/layouters/module_layouter.h"

class debug_layouter final : public module_layouter
{
public:
    debug_layouter(const module_context* const context);

    QString name() const override;
    QString description() const override;

    void layout() override;

    void add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& internal_nets, const QSet<u32>& local_io_nets, const QSet<u32>& global_io_nets) override;
    void remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& internal_nets, const QSet<u32>& local_io_nets, const QSet<u32>& global_io_nets) override;

    //void expand(const u32 from_gate, const u32 via_net, const u32 to_gate) override;
};

#endif // DEBUG_LAYOUTER_H
