#pragma once

#include "core/interface_base.h"
#include "netlist/boolean_function.h"
#include "z3++.h"

class netlist;
class gate;



class PLUGIN_API plugin_sat_solver : virtual public i_base
{
public:

    std::string get_name() const override;
    std::string get_version() const override;

    void initialize() override;

    void sat(const boolean_function& bf);
    void check_adder(const std::set<std::shared_ptr<gate>>& nl);

private:
    z3::expr convert_boolean_function_to_z3_expr(const boolean_function& bf);

    z3::context c;
};
