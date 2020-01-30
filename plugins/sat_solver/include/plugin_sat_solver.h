#pragma once

#include "core/interface_base.h"
#include "netlist/boolean_function.h"
#include "z3++.h"

class netlist;
class gate;
class net;

class PLUGIN_API plugin_sat_solver : virtual public i_base
{
public:
    std::string get_name() const override;
    std::string get_version() const override;

    void initialize() override;

    void sat(const boolean_function& bf);
    void check_adder(std::set<std::shared_ptr<net>> input_a, std::set<std::shared_ptr<net>> input_b, std::map<std::shared_ptr<net>, boolean_function> output_net_to_bf);

private:
    z3::expr convert_boolean_function_to_z3_expr(const boolean_function& bf, std::map<std::string, z3::expr> input2expr);

    z3::context c;
};
