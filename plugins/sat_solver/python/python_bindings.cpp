#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#include "plugin_sat_solver.h"

namespace py = pybind11;

#ifdef PYBIND11_MODULE
PYBIND11_MODULE(sat_solver, m)
{
    m.doc() = "hal sat_solver python bindings";
#else
PYBIND11_PLUGIN(sat_solver)
{
    py::module m("sat_solver", "hal sat_solver python bindings");
#endif    // ifdef PYBIND11_MODULE

    py::class_<plugin_sat_solver, std::shared_ptr<plugin_sat_solver>, i_base>(m, "sat_solver")
        .def_property_readonly("name", &plugin_sat_solver::get_name)
        .def("get_name", &plugin_sat_solver::get_name)
        .def_property_readonly("version", &plugin_sat_solver::get_version)
        .def("get_version", &plugin_sat_solver::get_version)
        ;

#ifndef PYBIND11_MODULE
    return m.ptr();
#endif    // PYBIND11_MODULE
}
