#pragma once

#include "core/interface_base.h"
#include "netlist/boolean_function.h"

class PLUGIN_API plugin_sat_solver : virtual public i_base
{
public:

    std::string get_name() const override;
    std::string get_version() const override;

    void initialize() override;

    void sat(const boolean_function& bf);

private:
    void convert_boolean_function_to_z3_expr(const boolean_function& bf);
};
