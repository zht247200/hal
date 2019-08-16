#ifndef DYNAMIC_CONTEXT_H
#define DYNAMIC_CONTEXT_H

#include "gui/graph_widget/contexts/graph_context.h"

class dynamic_context final : public graph_context
{
public:
    dynamic_context(const QString& name);

    bool contains_module(const u32 id) const override;
    bool contains_gate(const u32 id) const override;
    bool contains_net(const u32 id) const override;

    void add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);
    void remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);

    const QSet<u32>& modules() const;
    const QSet<u32>& gates() const;
    const QSet<u32>& nets() const;

    QString name() const;

private:
    static bool m_expand_all_nets;
    static bool m_expand_gnd_nets;
    static bool m_expand_vcc_nets;
    static bool m_expand_global_nets;

    void apply_changes() override;
    void evaluate_changes();

    QString m_name;

    QSet<u32> m_modules;
    QSet<u32> m_gates;
    QSet<u32> m_nets;

    QSet<u32> m_added_modules;
    QSet<u32> m_added_gates;
    QSet<u32> m_added_nets;

    QSet<u32> m_removed_modules;
    QSet<u32> m_removed_gates;
    QSet<u32> m_removed_nets;
};

#endif // DYNAMIC_CONTEXT_H
