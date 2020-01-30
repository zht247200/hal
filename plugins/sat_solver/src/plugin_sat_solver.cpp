#include "plugin_sat_solver.h"
#include "z3++.h"

#include "core/log.h"
#include "netlist/boolean_function.h"

#include "netlist/gate.h"
#include "netlist/net.h"

#include "netlist/netlist.h"

#include <map>

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
    log("trying SAT", bf.to_string());

    z3::solver s(c);

    // get all variable names and add them
    std::map<std::string, z3::expr> input2expr;

    for (const auto& var : bf.get_variables())
    {
        input2expr.insert(std::make_pair(var, c.bv_const(var.c_str(), 1)));
    }

    log("converting to z3 expr");

    z3::expr dnf = convert_boolean_function_to_z3_expr(bf, input2expr);

    s.add(dnf == c.bv_val(1, 1));

    auto check = s.check();

    if (check == z3::sat)
    {
        std::cout << "[+] dnf is satisfiable for: " << s.get_model() << std::endl;
    }
}

void plugin_sat_solver::check_adder(std::set<std::shared_ptr<net>> input_a, std::set<std::shared_ptr<net>> input_b, std::map<std::shared_ptr<net>, boolean_function> output_net_to_bf)
{
    //definition of model in z3

    // get all variable names and add them
    // inputs
    std::set<std::string> names;
    for (const auto& [net, bf] : output_net_to_bf)
    {
        for (const auto& var : bf.get_variables())
        {
            names.insert(var);
        }
    }

    std::map<std::string, z3::expr> input2expr;
    for (const auto& var : names)
    {
        input2expr.insert(std::make_pair(var, c.bv_const(var.c_str(), 1)));
    }

    std::vector<z3::expr> output_z3_bf;
    for (const auto& [net, bf] : output_net_to_bf)
    {
        z3::expr z3_bf = this->convert_boolean_function_to_z3_expr(bf, input2expr);
        output_z3_bf.push_back(z3_bf);
    }

    // for (const auto& z3_bf : output_z3_bf)
    // {
    //     std::cout << "[+] complete dnf: " << z3_bf.simplify() << std::endl;
    // }

    //z3::expr C = c.bv_const("C", 4);

    //C.extract(0, 0) = z3_bf.at(0);

    
    // concat alle outputs

    z3::expr A = c.bv_const("A", 4), B = c.bv_const("B", 4);

    auto adder_model = A + B;

    auto C = input2expr.at("14") + input2expr.at("10");

    auto expr = output_z3_bf.at(2);

    for (const auto& expr : output_z3_bf)
    {
        z3::solver s(c);
        s.add(expr != C.extract(0, 0));
        auto check = s.check();
        if (check == z3::unsat)
        {
            std::cout << "equal to stuff ..." << std::endl;
        }
    }
    //     std::cout << C.extract(0, 0).simplify() << std::endl;
    //     std::cout << expr.simplify() << std::endl;
    // //    for (const auto& expr : output_z3_bf) {
    //     z3::solver s(c);
    //     s.add(C.extract(0, 0) != expr);

    //     auto check = s.check();
    //     if (check == z3::unsat) {
    //         std::cout << "equal to stuff ... " << std::endl;
    //     } else {
    //         std::cout << s.get_model() << std::endl;
    //     }
}

z3::expr plugin_sat_solver::convert_boolean_function_to_z3_expr(const boolean_function& bf, std::map<std::string, z3::expr> input2expr)
{
    log("converting HAL-bf to z3::expr: {}", bf.to_string());

    std::vector<std::vector<std::pair<std::string, bool>>> dnf_vec = bf.get_dnf_clauses();

    std::vector<z3::expr> dnfs;

    for (const auto& clause : dnf_vec)
    {
        auto clause_expr = c.bv_val(1, 1);    // initialize to true as we logically AND everything in the clause
        for (const auto& [pin, negated] : clause)
        {
            clause_expr = clause_expr & ((negated) ? ~input2expr.at(pin) : input2expr.at(pin));
        }
        dnfs.emplace_back(clause_expr);
        //std::cout << "[+] adding " << clause_expr.simplify() << " to dnf ..." << std::endl;
    }

    z3::expr dnf = c.bv_val(0, 1);
    for (const auto& _dnf : dnfs)
    {
        dnf = dnf | _dnf;
    }

    return dnf.simplify();
}