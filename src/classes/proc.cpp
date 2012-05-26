#include "proc.hpp"
#include "symbol.hpp"
#include "class.hpp"
#include "../runtime.hpp"

namespace Mirb
{
	value_t Proc::call_with_options(value_t self_value, Tuple<Module> *scope, Proc *self, value_t block, size_t argc, value_t argv[])
	{
		Frame frame;

		frame.code = self->block;
		frame.obj = self_value;
		frame.name = self->name;
		frame.scope = scope;
		frame.block = block;
		frame.argc = argc;
		frame.argv = argv;
		frame.scopes = self->scopes;

		value_t result = call_frame(frame);

		if(!result)
			throw_current_exception();

		return result;
	}
	
	value_t Proc::call(Proc *self, value_t block, size_t argc, value_t argv[])
	{
		return call_with_options(self->self, self->scope, self, block, argc, argv);
	}

	void Proc::initialize()
	{
		context->proc_class = define_class("Proc", context->object_class);
		
		method<Arg::Self<Arg::Class<Proc>>, Arg::Block, Arg::Count, Arg::Values>(context->proc_class, "call", &call);
		method<Arg::Self<Arg::Class<Proc>>, Arg::Block, Arg::Count, Arg::Values>(context->proc_class, "[]", &call);
	}
};

