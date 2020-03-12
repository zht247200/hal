#include "netlist/gate_library/gate_library_parser/gate_library_parser.h"

gate_library_parser::gate_library_parser(std::stringstream& stream) : m_fs(stream)
{
    m_gate_lib = nullptr;
}

void gate_library_parser::add_gate_type_pin_widths(std::shared_ptr<gate_type> gt, std::map<std::string, u32> pin_widths)
{
    gt->m_pin_widths.insert(pin_widths.begin(), pin_widths.end());
}