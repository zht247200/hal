#ifndef MODULE_CONTEXT_H
#define MODULE_CONTEXT_H

#include "netlist/module.h"

#include "gui/graph_widget/contexts/graph_context.h"

class module_context final : public graph_context
{
public:
    module_context(const std::shared_ptr<const module> m);

    void add(const QSet<u32>& modules, const QSet<u32>& gates);
    void remove(const QSet<u32>& modules, const QSet<u32>& gates);
    //void net_change(const QSet<u32>& nets); // NECESSARY ???

    u32 get_id() const;

    const QSet<u32>& modules() const;
    const QSet<u32>& gates() const;
    const QSet<u32>& internal_nets() const;
    const QSet<u32>& global_io_nets() const;
    const QSet<u32>& local_io_nets() const;

private:
    void evaluate_changesets();
    void apply_changesets();

    const u32 m_id;

    QSet<u32> m_modules;
    QSet<u32> m_gates;
    QSet<u32> m_internal_nets;
    QSet<u32> m_global_io_nets;
    QSet<u32> m_local_io_nets;

    QSet<u32> m_added_modules;
    QSet<u32> m_added_gates;
    QSet<u32> m_added_internal_nets;
    QSet<u32> m_added_global_io_nets;
    QSet<u32> m_added_local_io_nets;

    QSet<u32> m_removed_modules;
    QSet<u32> m_removed_gates;
    QSet<u32> m_removed_internal_nets;
    QSet<u32> m_removed_global_io_nets;
    QSet<u32> m_removed_local_io_nets;
};

#endif // MODULE_CONTEXT_H
