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
        log_error("hdl_parser", "netlist_factory returned nullptr.");
        return nullptr;
    }

    for (auto& e : m_entities)
    {
        for (auto& inst : e.second._instances)
        {
            auto& port_assignments = inst.second._port_assignments;

            // instance of entity
            if (auto entity_it = m_entities.find(inst.second._type); entity_it != m_entities.end())
            {
                // fill in port width
                auto& entity_ports = entity_it->second._ports;

                for (auto& [port_name, assignment] : port_assignments)
                {
                    if (!assignment.first._is_bound_known)
                    {
                        if (auto port_it = entity_ports.find(port_name); port_it != entity_ports.end())
                        {
                            assignment.first.set_bounds(port_it->second.second._bounds);

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
                if (auto gate_it = gate_types.find(inst.second._type); gate_it != gate_types.end())
                {
                    auto pin_widths = gate_it->second->m_pin_bounds;

                    for (auto& [port_name, assignment] : port_assignments)
                    {
                        if (auto pin_it = pin_widths.find(port_name); pin_it != pin_widths.end())
                        {
                            assignment.first.set_bounds({pin_it->second});

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
                            log_error("hdl_parser", "port '{}' is no valid port of gate type '{}' in line {}.", port_name, gate_it->first, assignment.first._line_number);
                            return nullptr;
                        }
                    }
                }
                else
                {
                    log_error("hdl_parser", "type '{}' of instance '{}' is neither an entity, nor a gate type in line {}.", inst.second._type, inst.first, inst.second._line_number);
                    return nullptr;
                }
            }
        }
    }

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
        for (auto& i : e.second._instances)
        {
            no_instant.erase(i.second._type);
        }
    }

    if (no_instant.size() != 1)
    {
        log_error("hdl_parser", "Cannot auto-detect top module of netlist.");
        return nullptr;
    }

    return *(no_instant.begin());
}

std::string hdl_parser::get_unique_alias(const std::string& name)
{
    // if the name only appears once, we don't have to alias it
    if (m_name_occurrences[name] < 2)
    {
        return name;
    }

    // otherwise, add a unique string to the name
    return name + "_" + std::to_string(m_current_index[name]++);
}
