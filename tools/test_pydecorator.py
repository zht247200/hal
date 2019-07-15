import hal_py
import pydecorator
import pydecorator

netlist = netlist_factory.load_netlist("/home/sebbe/Desktop/hal/examples/fsm.v", "verilog", "EXAMPLE_LIB")

g = netlist.get_gate_by_id(1)
g.get_type()
netlist.create_gate("BUF", "TEST")
