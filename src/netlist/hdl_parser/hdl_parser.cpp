#include "netlist/hdl_parser/hdl_parser.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

hdl_parser::hdl_parser(std::stringstream& stream) : m_fs(stream)
{
    m_netlist = nullptr;
}

std::shared_ptr<netlist> hdl_parser::instantiate(const std::string& gate_library)
{
    // detect top module of netlist dynamically
    auto top_module = detect_top_module();

    // create empty netlist
    m_netlist        = netlist_factory::create_netlist(gate_library);
    auto& gate_types = m_netlist->get_gate_library()->get_gate_types();

    if (m_netlist == nullptr)
    {
        log_error("hdl_parser", "netlist_factory returned nullptr");
        return nullptr;
    }

    for (auto& e : m_entities)
    {
        for (auto& inst : e.second.instances)
        {
            auto& port_assignments = inst.second.port_assignments;

            // instance of entity
            if (auto entity_it = m_entities.find(inst.second.type); entity_it != m_entities.end())
            {
                // fill in port width
                auto& entity_ports = entity_it->second.ports;

                for (auto& [port_name, assignment] : port_assignments)
                {
                    if (!assignment.first._is_bound_known)
                    {
                        if (auto port_it = entity_ports.find(port_name); port_it != entity_ports.end())
                        {
                            assignment.first.set_bound(port_it->second.second._bound);

                            i32 left_size  = assignment.first.size();
                            i32 right_size = 0;
                            for (const auto& s : assignment.second)
                            {
                                right_size += s.size();
                            }

                            if (left_size != right_size)
                            {
                                log_error(
                                    "hdl_parser", "port assignment width mismatch: left side has size {} and right side has size {} in line {}.", left_size, right_size, assignment.first._line_number);
                                return nullptr;
                            }
                        }
                        else
                        {
                            log_error("hdl_parser", "port '{}' is no valid port of entity '{}' in line {}.", port_name, entity_it->first, assignment.first._line_number);
                            return nullptr;
                        }
                    }
                }

                // TODO instantiate instance
            }
            // instance of gate type
            else
            {
                if (auto gate_it = gate_types.find(inst.second.type); gate_it != gate_types.end())
                {
                    // TODO
                    auto test = get_gate_type_pin_width(gate_it->second);
                }
                else
                {
                    log_error("hdl_parser", "type '{}' of instance '{}' is neither an entity, nor a gate type in line {}.", inst.second.type, inst.first, inst.second.line_number);
                    return nullptr;
                }
            }
        }
    }

    // TODO match instances with entities and gate_types
    // TODO verify correctness of port assignments and fill in bounds
    // TODO verify correctness of assignments and fill in bounds

    return nullptr;
}

std::string hdl_parser::detect_top_module()
{
    std::set<std::string> no_instant;

    for (auto& e : m_entities)
    {
        no_instant.insert(e.first);
    }

    for (auto& e : m_entities)
    {
        for (auto& i : e.second.instances)
        {
            no_instant.erase(i.second.type);
        }
    }

    if (no_instant.size() != 1)
    {
        log_error("hdl_parser", "Cannot auto-detect top module of netlist.");
        return nullptr;
    }

    return *(no_instant.begin());
}

const std::map<std::string, u32> hdl_parser::get_gate_type_pin_width(const std::shared_ptr<const gate_type> gt) const
{
    return gt->m_pin_widths;
}