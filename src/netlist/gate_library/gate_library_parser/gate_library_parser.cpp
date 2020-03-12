#include "netlist/gate_library/gate_library_parser/gate_library_parser.h"

gate_library_parser::gate_library_parser(std::stringstream& stream) : m_fs(stream)
{
    m_gate_lib = nullptr;
}

void gate_library_parser::add_gate_type_pin_bounds(std::shared_ptr<gate_type> gt, std::map<std::string, std::pair<u32, u32>> pin_bounds)
{
    gt->m_pin_bounds.insert(pin_bounds.begin(), pin_bounds.end());
}