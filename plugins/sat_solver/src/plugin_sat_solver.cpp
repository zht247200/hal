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

    z3::solver s(c);

    z3::expr dnf = convert_boolean_function_to_z3_expr(bf);
    
    s.add(dnf == c.bv_val(1,1));

    auto check = s.check();

    if (check == z3::sat)
    {
        std::cout << "[+] dnf is satisfiable for: " << s.get_model() << std::endl;
    }
}

z3::expr plugin_sat_solver::convert_boolean_function_to_z3_expr(const boolean_function& bf)
{
    // simple example, will be replaced with devhoffmanns function bf.get_dnf_vec()
    const auto clause_1   = std::map<std::string, bool>({
        {"I0", true},
        {"I1", true},
        {"I2", false},
        {"I3", true},
    });
    const auto clause_2   = std::map<std::string, bool>({
        {"I0", false},
        {"I1", true},
        {"I2", false},
        {"I3", true},
        {"I4", true},
        {"I5", true},
    });

    std::vector<std::map<std::string, bool>> dnf_vec = {clause_1, clause_2};
    
    // get all variable names and add them
    std::map<std::string, z3::expr> input2expr;

    for (const auto& var : bf.get_variables())
    {
        input2expr.insert(std::make_pair(var, c.bv_const(var.c_str(), 1)));
    }

    std::vector<z3::expr> dnfs;

    for (const auto& clause : dnf_vec)
    {
        auto clause_expr = c.bv_val(1, 1);    // initialize to true as we logically AND everything in the clause
        for (const auto& [pin, negated] : clause)
        {
            clause_expr = clause_expr & ((negated) ? ~input2expr.at(pin) : input2expr.at(pin));
        }
        dnfs.emplace_back(clause_expr);
        std::cout << "[+] adding " << clause_expr.simplify() << " to dnf ..." << std::endl;
    }


    z3::expr dnf = c.bv_val(0, 1);
    for (const auto& _dnf : dnfs)
    {
        dnf = dnf | _dnf;
    }
    std::cout << "[+] complete dnf " << dnf.simplify() << " is_bool(): " << dnf.is_bool() << std::endl;

    return dnf;
}