import hal_py
import pydecorator
import pydecorator

netlist = netlist_factory.load_netlist("/home/sebbe/Desktop/hal/examples/fsm.v", "verilog", "EXAMPLE_LIB")
#netlist.create_gate("BUF", "testname")


log_info("NETLIST FUNCTIONS TESTING##################")

log_info("Calling function get_gate_by_id")
test_gate = netlist.get_gate_by_id(6)
log_info("Calling function get_net_by_id")
test_net = netlist.get_net_by_id(15)
log_info("Calling function get_gates")
netlist.get_gates("BUF")
log_info("Calling function get_nets")
netlist.get_nets()
log_info("Calling function create_module")
test_module = netlist.create_module("hey", netlist.get_top_module())
log_info("Calling function get_modules")
netlist.get_modules()
log_info("Calling function delete_module")
netlist.delete_module(test_module)
log_info("Calling function get_module_by_id")
netlist.get_module_by_id(1)
log_info("Calling function create_module")
test_module = netlist.create_module("hey", netlist.get_top_module())



log_info("GATE FUNCTIONS TESTING######################")


log_info("Calling function get_id")
test_gate.get_id()
log_info("Calling function get_name")
test_gate.get_name()
log_info("Calling function get_type")
test_gate.get_type()
log_info("Calling function get_data_by_key")
test_gate.get_data_by_key("generic","init")
log_info("Calling function set_data")
test_gate.set_data("category_0", "key_0", "data_type_0", "value_0")
log_info("Calling function set_name")
test_gate.set_name("DUBIDA")
log_info("Calling function get_predecessors")
test_gate.get_predecessors()
log_info("Calling function get_successors")
test_gate.get_successors()

log_info("NET FUNCTIONS TESTING##############################")
log_info("Calling function get_id")
test_net.get_id()
log_info("Calling function get_name")
test_net.get_name()
log_info("Calling function set_data")
test_net.set_data("category_0", "key_0", "data_type_0", "value_0")
log_info("Calling function get_src")
test_net.get_src()
log_info("Calling function get_dsts")
test_net.get_dsts()

log_info("MODULE FUNCTIONS TESTING###########################")
log_info("Calling function assign_gate")
test_module.assign_gate(test_gate)
log_info("Calling function gates")
test_module.get_gates()