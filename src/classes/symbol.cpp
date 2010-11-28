#include "symbol.hpp"
#include "string.hpp"
#include "../symbol-pool.hpp"
#include "../runtime.hpp"

namespace Mirb
{
	value_t Symbol::class_ref;
	
	Symbol *Symbol::from_string(const char *string)
	{
		return symbol_pool.get(string);
	}
	
	Symbol *Symbol::from_string(const std::string &string)
	{
		return symbol_pool.get(string);
	}
	
	Symbol *Symbol::from_char_array(CharArray &char_array)
	{
		return symbol_pool.get(char_array);
	}
	
	Symbol *Symbol::from_char_array(CharArray &&char_array)
	{
		CharArray &string = char_array;

		return symbol_pool.get(string);
	}
	
	mirb_compiled_block(symbol_to_s)
	{
		auto self = cast<Symbol>(obj);

		return self->string.to_string();
	}

	mirb_compiled_block(symbol_inspect)
	{
		auto self = cast<Symbol>(obj);

		CharArray string = ":" + self->string;

		return string.to_string();
	}

	void Symbol::initialize()
	{
		define_method(Symbol::class_ref, "to_s", symbol_to_s);
		define_method(Symbol::class_ref, "inspect", symbol_inspect);
	}
};

