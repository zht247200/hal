if "__decorated__" not in dir():
	__decorated__ = True

	from functools import wraps
	import hal_py

	##standard decoration
	def decorate(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			hal_py.log_info("pyfunction called: {}".format(message))
			return f(*args, **kwargs)
		return decorated

	def generic_generator(message, object_type, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			hal_py.log_info("Function: {}, {}-ID: {}".format(message, object_type, args[0].id))
			return result
		return decorated

	#######################Specific Functions

	#######Netlist Decorator
	def netlist_get_gate_by_id(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			hal_py.log_info("Function: {}, Gate-ID: {}".format(message, args[1]))
			return result
		return decorated

	def netlist_get_gates(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Gate-IDs: {"
			if len(result) == len(args[0].gates):
				log_string += "all}"
			elif len(result) == 0:
				log_string += "empty}"
			else:
				for g in result:
					log_string += str(g.id) + ", "
				log_string = log_string[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_get_net_by_id(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			hal_py.log_info("Function: {}, Net-ID: {}".format(message, args[1]))
			return result
		return decorated

	def netlist_get_nets(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Net-IDs: {"
			if len(result) == len(args[0].nets):
				log_string += "all}"
			elif len(result) == 0:
				log_string += "empty}"
			else:
				for g in result:
					log_string += str(g.id) + ", "
				log_string = log_string[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_create_module(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			hal_py.log_info("Function: {}, Module-ID: {}".format(message, result.get_id()))#make this a property call -> add property in python definitions
			return result
		return decorated

	def netlist_delete_module(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			gate_ids = args[1].get_gates()
			module_id = args[1].get_id()
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Module-ID: " + str(module_id) + ", Gate-IDs: {"
			if len(gate_ids) != 0:
				for g in gate_ids:
					log_string += str(g.id) + ", "
				log_string = log_string[:-2] + "}"
			else:
				log_string += "empty}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_get_module_by_id(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			gate_ids = result.get_gates()
			module_id = result.get_id()
			log_string = "Function: " + message + ", Module-ID: " + str(module_id) + ", Gate-IDs: {"
			if len(gate_ids) != 0:
				for g in gate_ids:
					log_string += str(g.id) + ", "
				log_string = log_string[:-2] + "}"
			else:
				log_string += "empty}"
			hal_py.log_info(log_string)
			return result
		return decorated

	def netlist_get_modules(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function called: " + message + " , Module-IDs: {"
			if len(result) == 0:
				log_string += "empty}"
			else:
				for mod in result:
					log_string += str(mod.get_id()) + ", "
				log_string = log_string[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	######### Gate Decorator
	def gate_get_predecessors_or_successors(message, pred_or_successor, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Gate-ID: " + str(args[0].id) + ", " + pred_or_successor+".-IDs: {"
			if len(result) != 0:
				for endpoint in result:
					log_string += str(endpoint.gate.id) + ", "
				log_string = log_string[:-2] + "}"
			else:
				log_string += "empty}"
			hal_py.log_info(log_string)
			return result
		return decorated
		

	########## Net Decorator
	def net_get_src(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			if result.gate is None:
				hal_py.log_info("Function: {}".format(message) + ", Gate-ID: {empty}")
			else:
				hal_py.log_info("Function: {}, Gate-ID: {}".format(message, result.gate.id))
			return result
		return decorated

	def net_get_dests(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Gate-IDs: {"
			if len(result) == 0:
				log_string += "empty}"
			else:
				for ep in result:
					log_string += str(ep.gate.id) + ", "
				log_string = log_string[:-2] + "}"
			hal_py.log_info(log_string)
			return result
		return decorated

	######## Module Decorator

	def module_assign_gate(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			hal_py.log_info("Function: {}, Module-ID: {}, Gate-ID: {}".format(message, args[0].get_id(), args[1].id))
			return result
		return decorated

	def module_gates(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			result = f(*args, **kwargs)
			log_string = "Function: " + message + ", Module-ID: " + str(args[0].get_id()) + ", Gate-IDs: {"
			if len(result) == 0:
				log_string += "empty}"
			else:
				for g in result:
					log_string += str(g.id) + ", "
				log_string = log_string[:-2] + "}"
			hal_py.log_info(log_string)
			return  result
		return decorated


	##### Decorate actual functions

	####### Netlist functions
	hal_py.netlist.get_gate_by_id = netlist_get_gate_by_id("netlist.get_gate_by_id", hal_py.netlist.get_gate_by_id)
	hal_py.netlist.get_gates = netlist_get_gates("netlist.get_gates", hal_py.netlist.get_gates)
	hal_py.netlist.get_net_by_id = netlist_get_net_by_id("netlist.get_net_by_id", hal_py.netlist.get_net_by_id)
	hal_py.netlist.get_nets = netlist_get_nets("netlist.get_nets", hal_py.netlist.get_nets)
	hal_py.netlist.create_module = netlist_create_module("netlist.create_module", hal_py.netlist.create_module)
	hal_py.netlist.delete_module = netlist_delete_module("netlist.delete_module", hal_py.netlist.delete_module)
	hal_py.netlist.get_module_by_id = netlist_get_module_by_id("netlist.get_module_by_id", hal_py.netlist.get_module_by_id)
	hal_py.netlist.get_modules = netlist_get_modules("netlist.get_modules", hal_py.netlist.get_modules)

	####### Gate Functions
	hal_py.gate.get_name = generic_generator("gate.get_name", "Gate", hal_py.gate.get_name)
	hal_py.gate.get_id = generic_generator("gate.get_id", "Gate", hal_py.gate.get_id)
	hal_py.gate.get_type = generic_generator("gate.get_type", "Gate", hal_py.gate.get_type)
	hal_py.gate.get_data_by_key = generic_generator("gate.get_data_by_key", "Gate", hal_py.gate.get_data_by_key)
	hal_py.gate.set_data = generic_generator("gate.set_data", "Gate", hal_py.gate.set_data)
	hal_py.gate.set_name = generic_generator("gate.set_name", "Gate", hal_py.gate.set_name)
	hal_py.gate.get_predecessors = gate_get_predecessors_or_successors("gate.get_predecessors", "predecessor", hal_py.gate.get_predecessors)
	hal_py.gate.get_successors = gate_get_predecessors_or_successors("gate.get_successors", "successor", hal_py.gate.get_successors)

	####### Net Functions
	hal_py.net.get_id = generic_generator("net.get_id", "Net", hal_py.net.get_id)
	hal_py.net.get_name = generic_generator("net.get_name", "Net", hal_py.net.get_name)
	hal_py.net.set_data = generic_generator("net.set_data", "Net", hal_py.net.set_data)
	hal_py.net.set_nama = generic_generator("net.set_name", "Net", hal_py.net.set_name)
	hal_py.net.get_src = net_get_src("net.get_src", hal_py.net.get_src)
	hal_py.net.get_dsts = net_get_dests("net.get_dsts", hal_py.net.get_dsts)

	####### Module Functions
	hal_py.module.assign_gate = module_assign_gate("module.assign_gate", hal_py.module.assign_gate)
	hal_py.module.get_gates = module_gates("module.gates", hal_py.module.get_gates)

else:
	hal_py.log_info("Already decorated. Not applying again.")

