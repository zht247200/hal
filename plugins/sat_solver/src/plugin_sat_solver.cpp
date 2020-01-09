#include "plugin_sat_solver.h"
#include "z3++.h"

using namespace z3;

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


void plugin_sat_solver::prove_example1() {
    std::cout << "prove_example1\n";
    
    context c;
    expr x      = c.int_const("x");
    expr y      = c.int_const("y");
    sort I      = c.int_sort();
    func_decl g = function("g", I, I);
    
    solver s(c);
    expr conjecture1 = implies(x == y, g(x) == g(y));
    std::cout << "conjecture 1\n" << conjecture1 << "\n";
    s.add(!conjecture1);
    if (s.check() == unsat) 
        std::cout << "proved" << "\n";
    else
        std::cout << "failed to prove" << "\n";

    s.reset(); // remove all assertions from solver s

    expr conjecture2 = implies(x == y, g(g(x)) == g(y));
    std::cout << "conjecture 2\n" << conjecture2 << "\n";
    s.add(!conjecture2);
    if (s.check() == unsat) {
        std::cout << "proved" << "\n";
    }
    else {
        std::cout << "failed to prove" << "\n";
        model m = s.get_model();
        std::cout << "counterexample:\n" << m << "\n";
        std::cout << "g(g(x)) = " << m.eval(g(g(x))) << "\n";
        std::cout << "g(y)    = " << m.eval(g(y)) << "\n";
    }
}