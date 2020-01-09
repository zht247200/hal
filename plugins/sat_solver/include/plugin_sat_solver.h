#pragma once

#include "core/interface_base.h"

class PLUGIN_API plugin_sat_solver : virtual public i_base
{
public:

    std::string get_name() const override;
    std::string get_version() const override;

    void initialize() override;

    void prove_example1();
};
