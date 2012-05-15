#include "nil-class.hpp"
#include "object.hpp"
#include "symbol.hpp"
#include "string.hpp"
#include "../runtime.hpp"

namespace Mirb
{
	value_t NilClass::to_s()
	{
		return String::from_literal("");
	}

	value_t NilClass::inspect()
	{
		return String::from_literal("nil");
	}
	
	value_t NilClass::nil()
	{
		return value_true;
	}
	
	void NilClass::initialize()
	{
		method(context->nil_class, "to_s", &to_s);
		method(context->nil_class, "inspect", &inspect);
		method(context->nil_class, "nil", &nil);

		set_const(context->object_class, Symbol::from_literal("NIL"), value_nil);
	};
};

