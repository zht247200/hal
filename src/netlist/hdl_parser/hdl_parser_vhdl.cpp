#include "netlist/hdl_parser/hdl_parser_vhdl.h"

#include "core/log.h"
#include "core/utils.h"

#include <iomanip>

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
        else if (m_token_stream.peek() == "architecture")
        {
            if (!parse_architecture())
            {
                return false;
            }
        }
        else
        {
            log_error("hdl_parser", "unexpected token '{}' in global scope in line {}", m_token_stream.peek().string, m_token_stream.peek().number);
            return false;
        }
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
    e._line_number = m_token_stream.peek().number;
    e._name        = m_token_stream.consume();
    m_token_stream.consume("is", true);

    auto next_token = m_token_stream.peek();
    while (next_token != "end")
    {
        if (next_token == "generic")
        {
            m_token_stream.consume_until(";");
            m_token_stream.consume(";", true);
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

static std::set<std::string> supported_directions = {"in", "out", "inout"};

bool hdl_parser_vhdl::parse_port_definitons(entity& e)
{
    // default port assignments are not supported
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
            signal s(line_number, name, bounds);
            e._ports.emplace(name, std::make_pair(direction, s));
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
            attribute = std::make_tuple(attr_type, "unknown", value);
        }
        else
        {
            attribute = std::make_tuple(attr_type, type_it->second, value);
        }

        if (attr_class == "entity")
        {
            e._entity_attributes.insert(attribute);
        }
        else if (attr_class == "label")
        {
            e._instance_attributes.emplace(attr_target, attribute);
        }
        else if (attr_class == "signal")
        {
            e._signal_attributes.emplace(attr_target, attribute);
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
        log_error("hdl_parser", "architecture refers to entity '{}', but no such entity exists.", entity_name.string);
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
        signal s(line_number, name, bounds);
        e._signals.emplace(name, s);
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
        // not in instance -> has to be a direct assignment
        else if (m_token_stream.find_next("<=") < m_token_stream.find_next(";"))
        {
            if (!parse_assign(e))
            {
                return false;
            }
        }
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

bool hdl_parser_vhdl::parse_assign(entity& e)
{
    auto line_number = m_token_stream.peek().number;
    auto left_str    = m_token_stream.extract_until("<=");
    m_token_stream.consume("<=", true);
    auto right_str = m_token_stream.extract_until(";");
    m_token_stream.consume(";", true);

    // extract assignments for each bit
    auto left_parts  = get_assignment_signals(e, left_str, true, false);
    auto right_parts = get_assignment_signals(e, right_str, false, false);

    // verify correctness
    if (left_parts.second == 0 || right_parts.second == 0)
    {
        // error already printed in subfunction
        return false;
    }

    if (left_parts.second != right_parts.second)
    {
        log_error("hdl_parser", "assignment width mismatch: left side has size {} and right side has size {} in line {}.", left_parts.second, right_parts.second, line_number);
        return false;
    }

    e._assignments[left_parts.first].insert(right_parts.first);

    return true;
}

bool hdl_parser_vhdl::parse_instance(entity& e)
{
    instance inst;

    inst._line_number = m_token_stream.peek().number;
    inst._name        = m_token_stream.consume();
    m_token_stream.consume(":", true);

    // remove prefix from type
    if (m_token_stream.peek() == "entity")
    {
        m_token_stream.consume("entity", true);
        inst._type = m_token_stream.consume();
        auto pos   = inst._type.find('.');
        if (pos != std::string::npos)
        {
            inst._type = inst._type.substr(pos + 1);
        }
    }
    else if (m_token_stream.peek() == "component")
    {
        m_token_stream.consume("component", true);
        inst._type = m_token_stream.consume();
    }
    else
    {
        inst._type    = m_token_stream.consume();
        auto low_type = core_utils::to_lower(inst._type);
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
            inst._type = inst._type.substr(prefix.size());
        }
    }

    if (m_token_stream.consume("generic"))
    {
        if (!parse_generic_assign(inst))
        {
            return false;
        }
    }

    if (m_token_stream.peek() == "port")
    {
        if (!parse_port_assign(e, inst))
        {
            return false;
        }
    }

    m_token_stream.consume(";", true);

    e._instances.emplace(inst._name, inst);

    return true;
}

bool hdl_parser_vhdl::parse_port_assign(entity& e, instance& inst)
{
    m_token_stream.consume("port", true);
    m_token_stream.consume("map", true);
    m_token_stream.consume("(", true);
    auto port_str = m_token_stream.extract_until(")");
    m_token_stream.consume(")", true);

    while (port_str.remaining() > 0)
    {
        auto left_str = port_str.extract_until("=>");
        port_str.consume("=>", true);
        auto right_str = port_str.extract_until(",");
        port_str.consume(",", port_str.remaining() > 0);    // last entry has no comma

        if (!right_str.consume("open"))
        {
            auto left_parts  = get_assignment_signals(e, left_str, true, true);
            auto right_parts = get_assignment_signals(e, right_str, false, true);

            if (left_parts.second == 0 || right_parts.second == 0)
            {
                // error already printed in subfunction
                return false;
            }

            inst._port_assignments.emplace(left_parts.first.at(0)._name, std::make_pair(left_parts.first.at(0), right_parts.first));
        }
    }

    return true;
}

bool hdl_parser_vhdl::parse_generic_assign(instance& inst)
{
    m_token_stream.consume("map", true);
    m_token_stream.consume("(", true);
    auto generic_str = m_token_stream.extract_until(")");
    m_token_stream.consume(")", true);

    while (generic_str.remaining() > 0)
    {
        std::string value, data_type;

        auto line_number = generic_str.peek().number;
        auto lhs         = generic_str.join_until("=>", "");
        generic_str.consume("=>", true);
        auto rhs = generic_str.join_until(",", "");
        generic_str.consume(",", generic_str.remaining() > 0);    // last entry has no comma

        // determine data type
        if ((core_utils::to_lower(rhs) == "true") || (core_utils::to_lower(rhs) == "false"))
        {
            value     = core_utils::to_lower(rhs);
            data_type = "boolean";
        }
        else if (core_utils::is_integer(rhs))
        {
            value     = rhs;
            data_type = "integer";
        }
        else if (core_utils::is_floating_point(rhs))
        {
            data_type = "floating_point";
        }
        else if (core_utils::ends_with(rhs, "s", true) || core_utils::ends_with(rhs, "sec", true) || core_utils::ends_with(rhs, "min", true) || core_utils::ends_with(rhs, "hr", true))
        {
            value     = rhs;
            data_type = "time";
        }
        else if (rhs.string[0] == '\"' && rhs.string.back() == '\"')
        {
            value     = rhs.string.substr(1, rhs.string.size() - 2);
            data_type = "string";
        }
        else if (rhs.string[0] == '\'' && rhs.string[2] == '\'')
        {
            value     = rhs.string.substr(1, 1);
            data_type = "bit_value";
        }
        else if (core_utils::starts_with(rhs, "b\"", true) || core_utils::starts_with(rhs, "o\"", true) || core_utils::starts_with(rhs, "x\"", true))
        {
            value = get_hex_from_literal(rhs);
            if (value.empty())
            {
                return false;
            }

            data_type = "bit_vector";
        }
        else
        {
            log_error("hdl_parser", "cannot identify data type of generic map value '{}' in instance '{}' in line {}", rhs.string, inst._name, line_number);
            return false;
        }

        inst._generic_assignments.emplace(lhs, std::make_pair(data_type, value));
    }

    return true;
}

// ###########################################################################
// ###################          Helper functions          ####################
// ###########################################################################

static std::map<std::string, u32> id_to_dim = {{"std_logic_vector", 1}, {"std_logic_vector2", 2}, {"std_logic_vector3", 3}};

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

std::pair<std::vector<hdl_parser_vhdl::signal>, i32> hdl_parser_vhdl::get_assignment_signals(entity& e, token_stream& signal_str, bool is_left_half, bool is_port_assignment)
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
    i32 size = 0;

    // (5) (1 - 5) & (1 - 5) & ...
    if (!is_left_half)
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
        auto signal_name_token  = part_stream.consume();
        i32 line_number         = signal_name_token.number;
        std::string signal_name = core_utils::to_lower(signal_name_token);
        std::vector<std::pair<i32, i32>> bounds;
        bool is_binary      = false;
        bool is_bound_known = true;

        // (2) NUMBER
        if (core_utils::starts_with(signal_name, "b\"") || core_utils::starts_with(signal_name, "o\"") || core_utils::starts_with(signal_name, "x\""))
        {
            if (is_left_half)
            {
                log_error("hdl_parser", "numeric value {} not allowed at this position in line {}.", signal_name, line_number);
                return {};
            }

            signal_name = get_bin_from_literal(signal_name_token);
            if (signal_name.empty())
            {
                return {{}, 0};
            }

            bounds    = {std::make_pair(signal_name.size() - 1, 0)};
            is_binary = true;
        }
        else
        {
            std::vector<std::pair<i32, i32>> reference_bounds;

            if (auto signal_it = e._signals.find(signal_name); signal_it != e._signals.end())
            {
                reference_bounds = signal_it->second._bounds;
            }
            else if (auto port_it = e._ports.find(signal_name); port_it != e._ports.end())
            {
                reference_bounds = port_it->second.second._bounds;
            }
            else
            {
                log_error("hdl_parser", "signal name '{}' is invalid in assignment in line {}.", signal_name, line_number);
                return {{}, 0};
            }

            // any bounds specified?
            if (part_stream.consume("("))
            {
                // (4) NAME(BEGIN_INDEX1 to/downto END_INDEX1, BEGIN_INDEX2 to/downto END_INDEX2, ...)
                if ((part_stream.find_next("downto", part_stream.position() + 2) != part_stream.position() + 2)
                    || (part_stream.find_next("to", part_stream.position() + 2) != part_stream.position() + 2))
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

                if (!is_in_bounds(bounds, reference_bounds))
                {
                    log_error("hdl_parser", "invalid bounds given for signal or port '{}' in line {}.", signal_name, line_number);
                    return {{}, 0};
                }
            }
            else
            {
                // (1) NAME *single-dimensional*
                if (is_port_assignment && is_left_half)
                {
                    is_bound_known = false;
                    bounds         = {};
                }
                else
                {
                    bounds = reference_bounds;
                }
            }
        }

        // create new signal for assign
        signal s(line_number, signal_name, bounds, is_binary, is_bound_known);
        size += s.size();
        result.push_back(s);
    }

    return std::make_pair(result, size);
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
    std::string value = core_utils::to_lower(core_utils::replace(value_token, "_", ""));

    std::string res;

    auto prefix        = value[0];
    std::string number = value.substr(2, value.find('\"') - 2);

    // parse number literal
    switch (prefix)
    {
        case 'b':
        {
            for (const auto& c : number)
            {
                if (c >= '0' && c <= '1')
                {
                    res += c;
                }
                else
                {
                    log_error("hdl_parser", "invalid character within binary number literal {} in line {}.", value, line_number);
                    return "";
                }
            }
            break;
        }

        case 'o':
        {
            for (const auto& c : number)
            {
                if (c >= '0' && c <= '7')
                {
                    res += oct_to_bin[c];
                }
                else
                {
                    log_error("hdl_parser", "invalid character within octal number literal {} in line {}.", value, line_number);
                    return "";
                }
            }
            break;
        }

        case 'h':
        {
            for (const auto& c : number)
            {
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
                {
                    res += hex_to_bin[c];
                }
                else
                {
                    log_error("hdl_parser", "invalid character within hexadecimal number literal {} in line {}.", value, line_number);
                    return "";
                }
            }
            break;
        }

        default:
        {
            log_error("hdl_parser", "invalid base '{}' within number literal {} in line {}.", prefix, value, line_number);
            return "";
        }
    }

    return res;
}

std::string hdl_parser_vhdl::get_hex_from_literal(token& value_token)
{
    auto line_number  = value_token.number;
    std::string value = core_utils::to_lower(core_utils::replace(value_token, "_", ""));

    u32 base;
    std::string res;

    auto prefix        = value[0];
    std::string number = value.substr(2, value.find('\"') - 2);
    i32 len;

    // select base
    switch (prefix)
    {
        case 'b':
        {
            if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '1'); }))
            {
                log_error("hdl_parser", "invalid character within binary number literal {} in line {}.", value, line_number);
                return "";
            }

            len  = number.size() + 3 / 4;
            base = 2;
            break;
        }

        case 'o':
        {
            if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '7'); }))
            {
                log_error("hdl_parser", "invalid character within octal number literal {} in line {}.", value, line_number);
                return "";
            }

            len  = number.size() + 1 / 2;
            base = 8;
            break;
        }

        case 'h':
        {
            if (!std::all_of(number.begin(), number.end(), [](const char& c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }))
            {
                log_error("hdl_parser", "invalid character within hexadecimal number literal {} in line {}.", value, line_number);
                return "";
            }

            len  = number.size();
            base = 16;
            break;
        }

        default:
        {
            log_error("hdl_parser", "invalid base '{}' within number literal {} in line {}.", prefix, value, line_number);
            return "";
        }
    }

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(len) << std::hex << stoull(number, 0, base);
    return ss.str();
}

bool hdl_parser_vhdl::is_in_bounds(const std::vector<std::pair<i32, i32>>& bounds, const std::vector<std::pair<i32, i32>>& reference_bounds) const
{
    if (bounds.size() != reference_bounds.size())
    {
        return false;
    }

    for (u32 i = 0; i < bounds.size(); i++)
    {
        i32 ref_max, ref_min;
        if (reference_bounds[i].first < reference_bounds[i].second)
        {
            ref_min = reference_bounds[i].first;
            ref_max = reference_bounds[i].second;
        }
        else
        {
            ref_min = reference_bounds[i].second;
            ref_max = reference_bounds[i].first;
        }

        if (!(((ref_min <= bounds[i].first) && (bounds[i].first <= ref_max)) && ((ref_min <= bounds[i].second) && (bounds[i].second <= ref_max))))
        {
            return false;
        }
    }

    return true;
}