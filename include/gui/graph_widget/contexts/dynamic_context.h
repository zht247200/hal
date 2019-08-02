#ifndef DYNAMIC_CONTEXT_H
#define DYNAMIC_CONTEXT_H

#include "gui/graph_widget/contexts/graph_context.h"

class dynamic_context : public graph_context
{
public:
    dynamic_context(const QString& name);

    const QSet<u32>& modules() const;
    const QSet<u32>& gates() const;
    const QSet<u32>& nets() const;

    QString name() const;

private:
    void apply_changes() override;

    QString m_name;

    QSet<u32> m_modules;
    QSet<u32> m_gates;
    QSet<u32> m_nets;

    bool m_extend_gnd;
    bool m_extend_vcc;
    bool m_extend_global;
};

#endif // DYNAMIC_CONTEXT_H
