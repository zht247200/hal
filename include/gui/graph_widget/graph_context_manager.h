#ifndef GRAPH_CONTEXT_MANAGER_H
#define GRAPH_CONTEXT_MANAGER_H

#include "def.h"

#include <QStringList>
#include <QVector>

class gate;
class module;
class net;

class cone_context;
class cone_layouter;
class cone_shader;
class dynamic_context;
class dynamic_layouter;
class dynamic_shader;
class module_context;
class module_layouter;
class module_shader;

class graph_context_manager
{
public:
    graph_context_manager();

    static void set_max_module_contexts(const int max);

    module_context* get_module_context(const u32 id);

    // RETRIEVAL AND REMOVAL SHOULD EITHER BE INDEX BASED OR NAMES NEED TO BE UNIQUE
    cone_context* add_cone_context(const QString& name);
    void delete_cone_context(const QString& name);
    cone_context* get_cone_context(const QString& name);
    QStringList cone_context_list() const;

    dynamic_context* add_dynamic_context(const QString& name);
    void delete_dynamic_context(const QString& name);
    dynamic_context* get_dynamic_context(const QString& name);
    QStringList dynamic_context_list() const;

    //void handle_module_created(const std::shared_ptr<module> m) const; // PRECACHING ???
    void handle_module_removed(const std::shared_ptr<module> m);
    void handle_module_name_changed(const std::shared_ptr<module> m) const;
    //void handle_module_parent_changed(const std::shared_ptr<module> m) const;
    void handle_module_submodule_added(const std::shared_ptr<module> m, const u32 added_module) const;
    void handle_module_submodule_removed(const std::shared_ptr<module> m, const u32 removed_module) const;
    void handle_module_gate_assigned(const std::shared_ptr<module> m, const u32 inserted_gate) const;
    void handle_module_gate_removed(const std::shared_ptr<module> m, const u32 removed_gate) const;

    //void handle_gate_created(const std::shared_ptr<gate> g) const;
    //void handle_gate_removed(const std::shared_ptr<gate> g) const;
    void handle_gate_name_changed(const std::shared_ptr<gate> g) const;

    void handle_net_created(const std::shared_ptr<net> n) const;
    void handle_net_removed(const std::shared_ptr<net> n) const;
    void handle_net_name_changed(const std::shared_ptr<net> n) const;
    void handle_net_src_changed(const std::shared_ptr<net> n) const;
    void handle_net_dst_added(const std::shared_ptr<net> n, const u32 dst_gate_id) const;
    void handle_net_dst_removed(const std::shared_ptr<net> n, const u32 dst_gate_id) const;

    cone_layouter* get_default_layouter(cone_context* const context) const;
    cone_shader* get_default_shader(cone_context* const context) const;

private:
    void debug_relayout_all() const;

    static int s_max_module_contexts;

    QVector<module_context*> m_module_contexts;
    QVector<cone_context*> m_cone_contexts;
    QVector<dynamic_context*> m_dynamic_contexts;
};

#endif // GRAPH_CONTEXT_MANAGER_H
