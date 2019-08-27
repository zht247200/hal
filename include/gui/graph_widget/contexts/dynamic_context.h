#ifndef DYNAMIC_CONTEXT_H
#define DYNAMIC_CONTEXT_H

#include "gui/graph_widget/contexts/graph_context.h"

class dynamic_layouter;
class dynamic_shader;

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
    enum class layouter
    {
        standard = 0
    };

    enum class shader
    {
        standard = 0
    };

    static dynamic_layouter* create_layouter(const layouter type, dynamic_context* const context);
    static dynamic_shader* create_shader(const shader type, dynamic_context* const context);

    static layouter s_default_layouter;
    static shader s_default_shader;

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

    layouter m_layouter_type;
};

#endif // DYNAMIC_CONTEXT_H
