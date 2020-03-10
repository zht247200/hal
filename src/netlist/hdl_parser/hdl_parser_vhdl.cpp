#include "netlist/hdl_parser/hdl_parser_vhdl.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/netlist.h"

#include "netlist/netlist_factory.h"

#include <queue>

hdl_parser_vhdl::hdl_parser_vhdl(std::stringstream& stream) : hdl_parser(stream)
{
}

// ###########################################################################
// ###########          Parse HDL into intermediate format          ##########
// ###########################################################################

bool hdl_parser_vhdl::parse()
{
    // tokenize file
    if (!tokenize())
    {
        return false;
    }

    // parse tokens into intermediate format
    try
    {
        if (!parse_tokens())
        {
            return false;
        }
    }
    catch (token_stream::token_stream_exception& e)
    {
        if (e.line_number != (u32)-1)
        {
            log_error("hdl_parser", "{} near line {}.", e.message, e.line_number);
        }
        else
        {
            log_error("hdl_parser", "{}.", e.message);
        }
        return false;
    }

    return true;
}

static bool is_digits(const std::string& str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);    // C++11
}

bool hdl_parser_vhdl::tokenize()
{
    std::vector<token> tmp_tokens;
    std::string delimiters = ",(): ;=><&";
    std::string current_token;
    u32 line_number = 0;

    std::string line;
    bool in_string = false;
    bool escaped   = false;
    while (std::getline(m_fs, line))
    {
        line_number++;
        if (line.find("--") != std::string::npos)
        {
            line = line.substr(0, line.find("--"));
        }
        for (char c : core_utils::trim(line))
        {
            if (c == '\\')
            {
                escaped = !escaped;
            }
            else if (!escaped && c == '"')
            {
                in_string = !in_string;
            }
            if (delimiters.find(c) == std::string::npos || escaped || in_string)
            {
                current_token += c;
            }
            else
            {
                if (!current_token.empty())
                {
                    if (tmp_tokens.size() > 1 && is_digits(tmp_tokens.at(tmp_tokens.size() - 2)) && tmp_tokens.at(tmp_tokens.size() - 1) == "." && is_digits(current_token))
                    {
                        tmp_tokens.pop_back();
                        tmp_tokens.back() += "." + current_token;
                    }
                    else
                    {
                        tmp_tokens.emplace_back(line_number, current_token, false);
                    }
                    current_token.clear();
                }
                if (c == '=' && tmp_tokens.at(tmp_tokens.size() - 1) == "<")
                {
                    tmp_tokens.at(tmp_tokens.size() - 1) = "<=";
                }
                else if (c == '=' && tmp_tokens.at(tmp_tokens.size() - 1) == ":")
                {
                    tmp_tokens.at(tmp_tokens.size() - 1) = ":=";
                }
                else if (c == '>' && tmp_tokens.at(tmp_tokens.size() - 1) == "=")
                {
                    tmp_tokens.at(tmp_tokens.size() - 1) = "=>";
                }
                else if (!std::isspace(c))
                {
                    tmp_tokens.emplace_back(line_number, std::string(1, c), false);
                }
            }
        }
        if (!current_token.empty())
        {
            tmp_tokens.emplace_back(line_number, current_token, false);
            current_token.clear();
        }
    }
    m_token_stream = token_stream(tmp_tokens, {"("}, {")"});
    return true;
}

bool hdl_parser_vhdl::parse_tokens()
{
    while (m_token_stream.remaining() > 0)
    {
        if (m_token_stream.peek() == "library" || m_token_stream.peek() == "use")
        {
            if (!parse_library())
            {
                return false;
            }
        }
        else if (m_token_stream.peek() == "entity")
        {
            if (!parse_entity())
            {
                return false;
            }
        }
        // else if (m_token_stream.peek() == "architecture")
        // {
        //     if (!parse_architecture())
        //     {
        //         return false;
        //     }
        // }
        // else
        // {
        //     log_error("hdl_parser", "unexpected token '{}' in global scope in line {}", m_token_stream.peek().string, m_token_stream.peek().number);
        //     return false;
        // }
    }

    return true;
}

bool hdl_parser_vhdl::parse_library()
{
    if (m_token_stream.peek() == "use")
    {
        m_token_stream.consume("use", true);
        auto lib = m_token_stream.consume().string;
        m_token_stream.consume(";", true);

        // remove specific import like ".all" but keep the "."
        lib = core_utils::trim(lib.substr(0, lib.rfind(".") + 1));
        m_libraries.insert(core_utils::to_lower(lib));
    }
    else
    {
        m_token_stream.consume_until(";");
        m_token_stream.consume(";", true);
    }
    return true;
}

bool hdl_parser_vhdl::parse_entity()
{
    entity e;

    m_token_stream.consume("entity", true);
    e.line_number = m_token_stream.peek().number;
    e.name        = m_token_stream.consume();
    m_token_stream.consume("is", true);

    auto next_token = m_token_stream.peek();
    while (next_token != "end")
    {
        if (next_token == "generic")
        {
            if (!parse_generic_definitions(e))
            {
                return false;
            }
        }
        else if (next_token == "port")
        {
            if (!parse_port_definitons(e))
            {
                return false;
            }
        }
        else if (next_token == "attribute")
        {
            if (!parse_attribute(e))
            {
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in entity defintion in line {}.", next_token.string, next_token.number);
            return false;
        }

        next_token = m_token_stream.peek();
    }

    m_token_stream.consume("end", true);
    m_token_stream.consume();
    m_token_stream.consume(";", true);

    return true;
}

static std::set<std::string> supported_directions = {"IN", "OUT", "INOUT"};

bool hdl_parser_vhdl::parse_generic_definitions(entity& e)
{
    // WARNING: default generic assignments are not supported
    m_token_stream.consume("generic", true);
    m_token_stream.consume("(", true);
    auto generic_str = m_token_stream.extract_until(")");

    while (generic_str.remaining() > 0)
    {
        // extract names
        do
        {
            e.generics.insert(generic_str.consume().string);
        } while (generic_str.consume(",", false));

        generic_str.consume_until(";");
        generic_str.consume(";", generic_str.remaining() > 0);
    }

    m_token_stream.consume(")", true);
    m_token_stream.consume(";", true);
    return true;
}

bool hdl_parser_vhdl::parse_port_definitons(entity& e)
{
    // WARNING: default port assignments are not supported
    m_token_stream.consume("port", true);
    m_token_stream.consume("(", true);
    auto port_str = m_token_stream.extract_until(")");

    while (port_str.remaining() > 0)
    {
        std::vector<std::string> port_names;
        std::set<signal> signals;

        auto line_number = port_str.peek().number;

        // extract names
        do
        {
            port_names.push_back(port_str.consume().string);
        } while (port_str.consume(",", false));

        port_str.consume(":", true);

        // extract direction
        auto direction = core_utils::to_lower(port_str.consume());
        if (supported_directions.find(direction) == supported_directions.end())
        {
            log_error("hdl_parser", "invalid direction '{}' for port declaration in line {}.", direction, line_number);
            return false;
        }

        // extract bounds
        auto bounds = parse_signal_bounds(port_str);
        if (bounds.empty())
        {
            // error already printed in subfunction
            return false;
        }

        port_str.consume(";", port_str.remaining() > 0);    // last entry has no semicolon, so no throw in that case

        for (const auto& name : port_names)
        {
            signal s;
            s.line_number = line_number;
            s.name        = name;
            s.bounds      = bounds;
            signals.insert(s);
        }

        if (direction == "IN")
        {
            e.in_ports.insert(signals.begin(), signals.end());
        }
        else if (direction == "OUT")
        {
            e.out_ports.insert(signals.begin(), signals.end());
        }
        else if (direction == "INOUT")
        {
            e.inout_ports.insert(signals.begin(), signals.end());
        }
    }

    m_token_stream.consume(")", true);
    m_token_stream.consume(";", true);

    return true;
}

bool hdl_parser_vhdl::parse_attribute(entity& e)
{
    std::tuple<std::string, std::string, std::string> attribute;
    u32 line_number = m_token_stream.peek().number;

    m_token_stream.consume("attribute", true);
    auto attr_type = core_utils::to_lower(m_token_stream.consume().string);

    if (m_token_stream.peek() == ":")
    {
        m_token_stream.consume(":", true);
        m_attribute_types[attr_type] = m_token_stream.join_until(";", " ");
        m_token_stream.consume(";", true);
    }
    else if (m_token_stream.peek() == "of" && m_token_stream.peek(2) == ":")
    {
        m_token_stream.consume("of", true);
        auto attr_target = m_token_stream.consume();
        m_token_stream.consume(":", true);
        auto attr_class = core_utils::to_lower(m_token_stream.consume());
        m_token_stream.consume("is", true);
        auto value = m_token_stream.join_until(";", " ").string;
        m_token_stream.consume(";", true);

        if (value[0] == '\"' && value.back() == '\"')
        {
            value = value.substr(1, value.size() - 2);
        }

        auto type_it = m_attribute_types.find(attr_type);
        if (type_it == m_attribute_types.end())
        {
            log_warning("hdl_parser", "attribute {} has unknown base type in line {}.", attr_type, line_number);
            attribute = std::make_tuple(attr_type, "UNKNOWN", value);
        }
        else
        {
            attribute = std::make_tuple(attr_type, type_it->second, value);
        }

        if (attr_class == "ENTITY")
        {
            e.entity_attributes.insert(attribute);
        }
        else if (attr_class == "LABEL")
        {
            e.instance_attributes.emplace(attr_target, attribute);
        }
        else if (attr_class == "SIGNAL")
        {
            e.signal_attributes.emplace(attr_target, attribute);
        }
        else
        {
            log_error("hdl_parser", "invalid attribute class '{}' in line {}.", attr_class, line_number);
            return false;
        }
    }
    else
    {
        log_error("hdl_parser", "malformed attribute defintion in line {}.", line_number);
        return false;
    }

    return true;
}

bool hdl_parser_vhdl::parse_architecture()
{
    m_token_stream.consume("architecture", true);
    m_token_stream.consume();
    m_token_stream.consume("of", true);

    auto entity_name = m_token_stream.consume();

    auto it = m_entities.find(entity_name);
    if (it == m_entities.end())
    {
        log_error("hdl_parser", "architecture refers to entity '{}', but no such entity exists.", entity_name);
        return false;
    }
    auto& e = it->second;

    m_token_stream.consume("is", true);

    return parse_architecture_header(e) && parse_architecture_body(e);
}

bool hdl_parser_vhdl::parse_architecture_header(entity& e)
{
    auto next_token = m_token_stream.peek();
    while (next_token != "begin")
    {
        if (next_token == "signal")
        {
            if (!parse_signal_definition(e))
            {
                return false;
            }
        }
        else if (next_token == "component")
        {
            // components are ignored
            m_token_stream.consume_until("end");
            m_token_stream.consume("end", true);
            m_token_stream.consume();
            m_token_stream.consume(";", true);
        }
        else if (next_token == "attribute")
        {
            if (!parse_attribute(e))
            {
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in architecture header in line {}", next_token.string, next_token.number);
            return false;
        }

        next_token = m_token_stream.peek();
    }

    return true;
}

bool hdl_parser_vhdl::parse_signal_definition(entity& e)
{
    std::vector<std::string> signal_names;

    m_token_stream.consume("signal", true);

    auto line_number = m_token_stream.peek().number;

    // extract names
    do
    {
        signal_names.push_back(m_token_stream.consume().string);
    } while (m_token_stream.consume(",", false));

    m_token_stream.consume(":", true);

    // extract bounds
    auto bounds = parse_signal_bounds(m_token_stream);
    if (bounds.empty())
    {
        // error already printed in subfunction
        return false;
    }

    m_token_stream.consume(";", true);

    for (const auto& name : signal_names)
    {
        signal s;
        s.line_number = line_number;
        s.name        = name;
        s.bounds      = bounds;
        e.signals.emplace(name, s);
    }

    return true;
}

bool hdl_parser_vhdl::parse_architecture_body(entity& e)
{
    m_token_stream.consume("begin", true);

    while (m_token_stream.peek() != "end")
    {
        // new instance found
        if (m_token_stream.peek(1) == ":")
        {
            if (!parse_instance(e))
            {
                return false;
            }
        }
        // TODO not in instance -> has to be a direct assignment
        // else if (m_token_stream.find_next("<=") < m_token_stream.find_next(";"))
        // {
        //     auto lhs = m_token_stream.extract_until("<=");
        //     m_token_stream.consume("<=", true);
        //     auto rhs = m_token_stream.extract_until(";");
        //     m_token_stream.consume(";", true);

        //     for (const auto& [name, value] : get_assignments(lhs, rhs))
        //     {
        //         e.direct_assignments[name] = value;
        //     }
        // }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in architecture body in line {}", m_token_stream.peek().string, m_token_stream.peek().number);
            return false;
        }
    }

    m_token_stream.consume("end", true);
    m_token_stream.consume();
    m_token_stream.consume(";", true);
    return true;
}

bool hdl_parser_vhdl::parse_instance(entity& e)
{
    instance inst;

    inst.line_number = m_token_stream.peek().number;
    inst.name        = m_token_stream.consume();
    m_token_stream.consume(":", true);

    // remove prefix from type
    if (m_token_stream.peek() == "entity")
    {
        m_token_stream.consume("entity", true);
        inst.type = m_token_stream.consume();
        auto pos  = inst.type.find('.');
        if (pos != std::string::npos)
        {
            inst.type = inst.type.substr(pos + 1);
        }
    }
    else if (m_token_stream.peek() == "component")
    {
        m_token_stream.consume("component", true);
        inst.type = m_token_stream.consume();
    }
    else
    {
        inst.type     = m_token_stream.consume();
        auto low_type = core_utils::to_lower(inst.type);
        std::string prefix;

        // find longest matching library prefix
        for (const auto& lib : m_libraries)
        {
            if (lib.size() > prefix.size() && core_utils::starts_with(low_type, lib))
            {
                prefix = lib;
            }
        }

        // remove prefix
        if (!prefix.empty())
        {
            inst.type = inst.type.substr(prefix.size());
        }
    }

    if (m_token_stream.peek() == "generic")
    {
        m_token_stream.consume("generic", true);
        m_token_stream.consume("map", true);
        m_token_stream.consume("(", true);
        auto generic_map = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (generic_map.remaining() > 0)
        {
            auto lhs = generic_map.join_until("=>", " ");
            generic_map.consume("=>", true);
            auto rhs = generic_map.join_until(",", " ");
            generic_map.consume(",", generic_map.remaining() > 0);    // last entry has no comma

            inst.generic_assignments.emplace(lhs, rhs);
        }
    }

    if (m_token_stream.peek() == "port")
    {
        m_token_stream.consume("port", true);
        m_token_stream.consume("map", true);
        m_token_stream.consume("(", true);
        auto port_map = m_token_stream.extract_until(")");
        m_token_stream.consume(")", true);

        while (port_map.remaining() > 0)
        {
            auto port_lhs = port_map.extract_until("=>");
            port_map.consume("=>", true);
            auto port_rhs = port_map.extract_until(",");
            port_map.consume(",", port_map.remaining() > 0);    // last entry has no comma

            // TODO support for "OPEN"
            auto processed_lhs = get_assignment_signals(port_lhs, false, false);
            auto processed_rhs = get_assignment_signals(port_rhs, true, true);

            if (processed_lhs.size() != 1 || processed_rhs.empty())
            {
                // error already printed in subfunction
                return false;
            }

            inst.port_assignments.emplace(processed_lhs.at(0), processed_rhs);
        }
    }

    m_token_stream.consume(";", true);

    e.instances.emplace(inst.name, inst);

    return true;
}

// ###########################################################################
// ###################          Helper functions          ####################
// ###########################################################################

static std::map<std::string, u32> id_to_dim = {{"STD_LOGIC_VECTOR", 1}, {"STD_LOGIC_VECTOR2", 2}, {"STD_LOGIC_VECTOR3", 3}};

std::vector<std::pair<i32, i32>> hdl_parser_vhdl::parse_signal_bounds(token_stream& signal_str)
{
    std::vector<std::pair<i32, i32>> bounds;
    auto line_number = signal_str.peek().number;

    // extract bounds
    auto type_str = signal_str.extract_until(":=");    // default assignment will be ignored for now

    if (type_str.size() == 1)
    {
        type_str.consume("std_logic", true);
        bounds.emplace_back(-1, -1);
    }
    else
    {
        auto type_name = core_utils::to_lower(type_str.consume());
        type_str.consume("(");
        auto bound_str = type_str.extract_until(")");
        type_str.consume(")");

        while (bound_str.remaining() > 0)
        {
            auto bound = bound_str.extract_until(",");
            bound_str.consume(",");

            if (bound.size() != 3)
            {
                log_error("hdl_parser", "range '{}' could not be parsed (line {}).", bound.join(" ").string, line_number);
                return {};
            }

            if (bound.at(1) == "downto")
            {
                bounds.emplace_back(std::stoi(bound.at(2)), std::stoi(bound.at(0)));
            }
            else if (bound.at(1) == "to")
            {
                bounds.emplace_back(std::stoi(bound.at(0)), std::stoi(bound.at(2)));
            }
            else
            {
                log_error("hdl_parser", "range '{}' could not be parsed (line {}).", bound.join(" ").string, line_number);
                return {};
            }

            // compare expected dimension with the dimension actually found
            auto tmp_type_name = core_utils::to_lower(type_name);
            if (id_to_dim.find(tmp_type_name) != id_to_dim.end())
            {
                auto dimension = id_to_dim.at(tmp_type_name);

                if (bounds.size() != dimension)
                {
                    log_error("hdl_parser", "dimension-bound mismatch in line {} : expected {}, got {}.", line_number, dimension, bounds.size());
                    return {};
                }
            }
        }
    }

    return bounds;
}

std::vector<hdl_parser_vhdl::signal> hdl_parser_vhdl::get_assignment_signals(token_stream& signal_str, bool allow_numerics, bool allow_concatenation)
{
    // PARSE ASSIGNMENT
    //   assignment can currently be one of the following:
    //   (1) NAME *single-dimensional*
    //   (2) NUMBER
    //   (3) NAME(INDEX1, INDEX2, ...)
    //   (4) NAME(BEGIN_INDEX1 to/downto END_INDEX1, BEGIN_INDEX2 to/downto END_INDEX2, ...)
    //   (5) (1 - 4) & (1 - 4) & ...

    std::vector<signal> result;
    std::vector<token_stream> parts;

    // (5) (1 - 5) & (1 - 5) & ...
    if (allow_concatenation)
    {
        do
        {
            parts.push_back(signal_str.extract_until("&"));
        } while (signal_str.consume("&", false));
    }
    else
    {
        if (signal_str.find_next("&") != token_stream::END_OF_STREAM)
        {
            log_error("hdl_parser", "concatenation is not allowed at this position in line {}.", signal_str.peek().number);
            return {};
        }
        parts.push_back(signal_str);
    }

    for (auto& part_stream : parts)
    {
        auto signal_name = part_stream.consume();

        // (2) NUMBER
        if (core_utils::starts_with(signal_name, "B\"", true) || core_utils::starts_with(signal_name, "O\"", true) || core_utils::starts_with(signal_name, "X\""))
        {
            if (!allow_numerics)
            {
                log_error("hdl_parser", "numeric value {} not allowed at this position in line {}.", signal_name.string, signal_name.number);
                return {};
            }

            signal s;
            s.line_number = signal_name.number;
            s.binary      = true;
            s.name        = get_bin_from_literal(signal_name);
            s.bounds      = {std::make_pair(s.name.size() - 1, 0)};

            result.push_back(s);

            continue;
        }

        // create new signal for assign
        signal s;
        s.line_number = signal_name.number;
        s.name        = signal_name.string;

        std::vector<std::pair<i32, i32>> bounds;

        // any bounds specified?
        if (part_stream.consume("("))
        {
            // (4) NAME(BEGIN_INDEX1 to/downto END_INDEX1, BEGIN_INDEX2 to/downto END_INDEX2, ...)
            if ((part_stream.find_next("downto", part_stream.position() + 2) != part_stream.position() + 2) || (part_stream.find_next("to", part_stream.position() + 2) != part_stream.position() + 2))
            {
                do
                {
                    i32 left_bound = std::stoi(part_stream.consume());
                    part_stream.consume();
                    i32 right_bound = std::stoi(part_stream.consume());

                    bounds.emplace_back(left_bound, right_bound);
                } while (part_stream.consume(",", false));
                part_stream.consume(")", true);
            }
            // (3) NAME(INDEX1, INDEX2, ...)
            else
            {
                do
                {
                    i32 index = std::stoi(part_stream.consume());

                    bounds.emplace_back(index, index);
                } while (part_stream.consume(","));
                part_stream.consume(")", true);
            }
        }
        else
        {
            // (1) NAME *single-dimensional*
            bounds.emplace_back(-1, -1);
        }

        s.bounds = bounds;

        result.push_back(s);
    }

    return result;
}

static std::map<char, std::string> oct_to_bin = {{'0', "000"}, {'1', "001"}, {'2', "010"}, {'3', "011"}, {'4', "100"}, {'5', "101"}, {'6', "110"}, {'7', "111"}};
static std::map<char, std::string> hex_to_bin = {{'0', "0000"},
                                                 {'1', "0001"},
                                                 {'2', "0010"},
                                                 {'3', "0011"},
                                                 {'4', "0100"},
                                                 {'5', "0101"},
                                                 {'6', "0110"},
                                                 {'7', "0111"},
                                                 {'8', "1000"},
                                                 {'9', "1001"},
                                                 {'a', "1010"},
                                                 {'b', "1011"},
                                                 {'c', "1100"},
                                                 {'d', "1101"},
                                                 {'e', "1110"},
                                                 {'f', "1111"}};

std::string hdl_parser_vhdl::get_bin_from_literal(token& value_token)
{
    auto line_number  = value_token.number;
    std::string value = core_utils::to_lower(core_utils::trim(core_utils::replace(value_token, "_", "")));

    std::string res;

    auto prefix        = value[0];
    std::string number = value.substr(2, value.find('\"') - 2);

    // parse number literal
    switch (prefix)
    {
        case 'b':
            for (const auto& c : number)
            {
                if (c >= '0' && c <= '1')
                {
                    res += c;
                }
                else
                {
                    log_error("hdl_parser", "invalid character '{}' within binary number literal {} in line {}.", c, value, line_number);
                    return "";
                }
            }
            break;

        case 'o':
            for (const auto& c : number)
            {
                if (c >= '0' && c <= '7')
                {
                    res += oct_to_bin[c];
                }
                else
                {
                    log_error("hdl_parser", "invalid character '{}' within octal number literal {} in line {}.", c, value, line_number);
                    return "";
                }
            }
            break;

        case 'h':
            for (const auto& c : number)
            {
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
                {
                    res += hex_to_bin[c];
                }
                else
                {
                    log_error("hdl_parser", "invalid character '{}' within hexadecimal number literal {} in line {}.", c, value, line_number);
                    return "";
                }
            }
            break;

        default:
            log_error("hdl_parser", "invalid base '{}' within number literal {} in line {}.", prefix, value, line_number);
            return "";
    }

    return res;
}