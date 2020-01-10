#include "plugin_sat_solver.h"
#include "z3++.h"

#include "core/log.h"
#include "netlist/boolean_function.h"

extern std::shared_ptr<i_base> get_plugin_instance()
{
    return std::make_shared<plugin_sat_solver>();
}

std::string plugin_sat_solver::get_name() const
{
    return std::string("sat_solver");
}

std::string plugin_sat_solver::get_version() const
{
    return std::string("0.1");
}

void plugin_sat_solver::initialize()
{
}

void plugin_sat_solver::sat(const boolean_function& bf)
{
    log("SAT for {}", bf.to_string());

    z3::context c;
    z3::solver s();

    convert_boolean_function_to_z3_expr(bf.to_dnf());
    //s.add(your formula here);
    //if (s.check() == z3::sat) then std::cout << s.model() << std::endl;
}

void plugin_sat_solver::convert_boolean_function_to_z3_expr(const boolean_function& bf)
{
    z3::context c;

    std::map<std::string, bool> clause_1;
    std::map<std::string, bool> clause_2;

    clause_1.insert(std::make_pair("I0", true));
    clause_1.insert(std::make_pair("I1", true));
    clause_1.insert(std::make_pair("I2", false));
    clause_1.insert(std::make_pair("I3", true));

    clause_2.insert(std::make_pair("I0", false));
    clause_2.insert(std::make_pair("I1", true));
    clause_2.insert(std::make_pair("I2", false));
    clause_2.insert(std::make_pair("I3", true));
    clause_2.insert(std::make_pair("I4", true));
    clause_2.insert(std::make_pair("I5", true));

    std::vector<std::map<std::string, bool>> dnf = {clause_1, clause_2};

    z3::solver s(c);

    // get all variable names and add them
    for (const auto& var : bf.get_variables())
    {
        c.bool_const(var.c_str());
    }

    z3::expr bf_z3(c);

    for (const auto& clause : dnf)
    {
        z3::expr clause_z3(c);
        for (const auto& var_it : clause)
        {
            auto var = var_it.first;
            auto value = var_it.second;
            // todo: add var to clause as &
        }

        //todo: or clause to bf_z3
    }

    std::cout << bf_z3.simplify() << std::endl;


    return;
}
