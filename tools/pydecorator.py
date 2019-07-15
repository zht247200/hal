if "__decorated__" not in dir():
	__decorated__ = True

	from functools import wraps
	import hal_py

	def decorate(message, f):
		@wraps(f)
		def decorated(*args, **kwargs):
			hal_py.log_info("pyfunction called: {}".format(message))
			return f(*args, **kwargs)
		return decorated


	hal_py.gate.get_type = decorate("gate.get_type", hal_py.gate.get_type)

	hal_py.netlist.create_gate = decorate("netlist.create_gate", hal_py.netlist.create_gate)


else:
	hal_py.log_info("Already decorated. Not applying again.")

