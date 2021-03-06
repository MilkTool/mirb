#include "string.hpp"
#include "symbol.hpp"
#include "array.hpp"
#include "fixnum.hpp"
#include "regexp.hpp"
#include "range.hpp"
#include "../number.hpp"
#include "../runtime.hpp"
#include "../internal.hpp"

namespace Mirb
{
	value_t String::from_symbol(Symbol *symbol)
	{
		return new (collector) String(symbol->string);
	}
	
	String *String::get(const CharArray &string)
	{
		return new (collector) String(string);
	}
			
	value_t String::from_string(const char *c_str)
	{
		return new (collector) String((const char_t *)c_str, std::strlen(c_str));
	}
	
	value_t String::rb_each_char(String *self, value_t block)
	{
		OnStack<2> os(self, block);

		for(size_t i = 0; i < self->string.size(); ++i)
			yield(block, CharArray(self->string[i]).to_string());
		
		return self;
	}
	
	value_t String::inspect(String *self)
	{
		CharArray result = "\"";
		
		self->string.each_char([&](char_code_t c, size_t) {
			switch(c)
			{
				case '\\':
					result += "\\\\";
					break;
					
				case '"':
					result += "\\\"";
					break;

				case '#':
					result += "\\#";
					break;

				case '\n':
					result += "\\n";
					break;

				case '\r':
					result += "\\r";
					break;

				default:
				{
					if(c < 32 || c >= 127)
						result +=  "\\x" + Number((intptr_t)c).to_string(16).rjust(2, "0");
					else
						result += c;
					break;
				}
			}
		});
		
		return (result + "\"").to_string();
	}
	
	value_t String::concat(String *self, String *other)
	{
		self->string.append(other->string);

		return self;
	}
	
	value_t String::add(String *self, String *other)
	{
		return String::get(self->string + other->string);
	}
	
	value_t split(String *self, value_t sep)
	{
		auto result = Array::allocate();

		CharArray sep_str;

		if(sep)
		{
			auto regexp = try_cast<Regexp>(sep);

			if(regexp)
			{
				regexp->split(self->string, [&](const CharArray &data) { result->vector.push(data.to_string()); }, [&](size_t, size_t) {});

				return result;
			}
			else
				sep_str = raise_cast<String>(sep)->string;
		}
		else
			sep_str = " ";

		if(sep_str.size())
		{
			self->string.split([&](const CharArray &match) {
				if(sep_str != " " || match.size())
					result->vector.push(match.to_string());
			}, sep_str);
		}
		else
			self->string.each_char([&](char_t c, size_t) { result->vector.push(CharArray(c).to_string()); });

		return result;
	}
	
	value_t String::to_s(value_t self)
	{
		return self;
	}
	
	value_t String::to_i(String *self, intptr_t base)
	{
		if(base == Fixnum::undef)
			base = 10;

		if(base < 2 || base > 64)
			raise(context->argument_error, "Base must be in 2..64");

		return Number(self->string, base).to_value();
	}
	
	value_t chomp(String *self, String *seperator)
	{
		if(seperator)
			return self->string.chomp(seperator->string).to_string();
		else
			return self->string.chomp().to_string();
	}
	
	value_t chomp_ex(String *self, String *seperator)
	{
		CharArray chomp;
		
		if(seperator)
			chomp = self->string.chomp(seperator->string);
		else
			chomp = self->string.chomp();

		if(self->string == chomp)
			return value_nil;
		else
		{
			self->string = chomp;
			return self;
		}
	}
	
	value_t match(value_t self, value_t regexp)
	{
		if(!of_type<Regexp>(regexp))
		{
			regexp = call(context->regexp_class, "new", regexp);

			type_error(regexp, context->regexp_class);
		}

		return call(regexp, "match", self);
	}

	value_t String::pattern(String *self, value_t other)
	{
		auto regexp = try_cast<Regexp>(other);

		if(!regexp)
			return call(other, "=~", self);
		
		int ovector[Regexp::vector_size];
		
		int result = regexp->match(self->string, ovector, 0);

		if(result <= 0)
			return value_nil;

		return Fixnum::from_int(ovector[0]);
	}

	value_t String::equal(String *self, value_t other)
	{
		auto string = try_cast<String>(other);

		if(!string)
			return value_false;
		else
			return Value::from_bool(self->string == string->string);
	}
	
	value_t String::downcase(String *self)
	{
		CharArray result(self->string);

		result.downcase();

		return result.to_string();
	}

	value_t String::downcase_self(String *self)
	{
		if(self->string.downcase())
			return self;
		else
			return value_nil;
	}
	
	value_t String::upcase(String *self)
	{
		CharArray result(self->string);

		result.upcase();

		return result.to_string();
	}

	value_t String::upcase_self(String *self)
	{
		if(self->string.upcase())
			return self;
		else
			return value_nil;
	}
	
	value_t String::rb_get(String *self, value_t index, value_t size)
	{
		if(size)
		{
			size_t i;

			if(!map_index(Fixnum::to_int(raise_cast<Type::Fixnum>(index)), self->string.size(), i))
				return String::get("");

			auto length = Fixnum::to_int(raise_cast<Type::Fixnum>(size));
			
			if(length < 0)
				return value_nil;

			return self->string.copy(i, length).to_string();
		}

		if(Value::is_fixnum(index))
		{
			size_t i;

			if(!map_index(Fixnum::to_int(index), self->string.size(), i))
				return value_nil;

			return String::get(self->string[i]);
		}
		else if(of_type<Range>(index))
		{
			size_t start;
			size_t length;

			auto range = cast<Range>(index);

			range->convert_to_index(start, length, self->string.size());

			return String::get(self->string.copy(start, length));
		}
		else
			type_error(index, "Range or Fixnum");
	}
	
	value_t String::ljust(String *self, size_t length, String *other)
	{
		if(other && !other->string.size())
			raise(context->argument_error, "Empty padding string");

		return self->string.ljust(length, other ? other->string : " ").to_string();
	}
	
	value_t String::rjust(String *self, size_t length, String *other)
	{
		if(other && !other->string.size())
			raise(context->argument_error, "Empty padding string");

		return self->string.rjust(length, other ? other->string : " ").to_string();
	}
	
	value_t to_sym(String *self)
	{
		return Symbol::get(self->string);
	}

	value_t String::compare(String *self, String *other)
	{
		for(size_t i = 0; true; ++i)
		{
			bool self_empty = self->string.size() - i == 0;
			bool other_empty = other->string.size() - i == 0;

			if(self_empty)
				return other_empty ? Fixnum::from_int(0) : Fixnum::from_int(-1);
			else if(other_empty)
				return Fixnum::from_int(1);
			
			char_t self_char = self->string[i];
			char_t other_char = other->string[i];
			
			if(self_char > other_char)
				return Fixnum::from_int(1);

			if(self_char < other_char)
				return Fixnum::from_int(-1);
		}
	}
	
	value_t String::tr(String *self, String *from, String *to)
	{
		CharArray result = self->string;

		if(!to->string.size() || !from->string.size())
			return result.to_string();

		result.each_char([&](char_t &input, size_t) {
			from->string.each_char([&](char_t &from, size_t index) {
				if(input == from)
				{
					if(index >= to->string.size())
						input = to->string[to->string.size() - 1];
					else
						input = to->string[index];
				}
			});
		});

		return result.to_string();
	}
	
	value_t String::include(String *self, String *sub)
	{
		size_t index;

		return Value::from_bool(self->string.index_of(sub->string, index));
	}

	value_t String::sprintf(String *self, value_t input)
	{
		CharArray s = self->string.c_str();
		CharArray result;

		OnStackString<2> oss(s, result);

		auto array = cast_array(input);

		OnStack<1> os(array);
		
		size_t i = 0;
		size_t argi = 0;
		size_t prev = 0;
		size_t start;
		int padding;
		char pad_char;

		auto arg = [&]() -> value_t {
			if(argi >= array->size())
				raise(context->argument_error, "Too few arguments");

			return array->get(argi++);
		};

		auto push = [&](size_t to) {
			result += s.copy(prev, to - prev);
		};
		
		auto write = [&](value_t value) {
			push(start);
			auto str = raise_cast<String>(call(value, "to_s"));
			result += str->string.rjust(padding, pad_char);
			prev = ++i;
		};

		while(i < s.size() - 1)
		{
			if(s[i] == '%')
			{
				start = i;

				++i;

				pad_char = ' ';

				if(s[i] == '0')
				{
					++i;
					pad_char = '0';
				}

				size_t width = i;

				while(s[i] >= '0' && s[i] <= '9')
					++i;

				padding = s.copy(width, i - width).to_i();

				switch(s[i])
				{
					case 'd':
						write(raise_cast<Type::Fixnum>(call(arg(), "to_i")));
						break;

					case 'f':
						write(raise_cast<Type::Float>(call(arg(), "to_f")));
						break;

					default:
						break;
				};
			}
			else
				++i;
		}

		push(i);

		return result.to_string();
	};
	
	value_t String::empty(String *self)
	{
		return Value::from_bool(self->string.size() == 0);
	}
	
	value_t String::length(String *self)
	{
		return Fixnum::from_size_t(self->string.size());
	}
	
	value_t String::ord(String *self)
	{
		if(self->string.size() < 1)
			raise(context->argument_error, "Can't get ordinal of a multicharacter string");

		return Fixnum::from_size_t(self->string[0]);
	}
	
	value_t String::gsub(String *self, Regexp *pattern, String *replacement)
	{
		bool changed;

		return Regexp::gsub(self->string, pattern, replacement->string, changed).to_string();
	}
	
	value_t String::gsub_ex(String *self, Regexp *pattern, String *replacement)
	{
		bool changed;

		self->string = Regexp::gsub(self->string, pattern, replacement->string, changed);

		return changed ? self : value_nil;
	}
	
	void String::initialize()
	{
		internal_allocator<String, &Context::string_class>();
		
		method<Self<Value>, Value, &match>(context->string_class, "match");
		method<Self<Value>, &to_s>(context->string_class, "to_s");
		method<Self<String>, Optional<Arg::Fixnum>, &to_i>(context->string_class, "to_i");
		method<Self<String>, Optional<String>, &chomp>(context->string_class, "chomp");
		method<Self<String>, Optional<String>, &chomp_ex>(context->string_class, "chomp!");
		method<Self<String>, Value, &pattern>(context->string_class, "=~");
		method<Self<String>, Value, &sprintf>(context->string_class, "%");
		method<Self<String>, String, &include>(context->string_class, "include?");
		method<Self<String>, Regexp, String, &gsub>(context->string_class, "gsub");
		method<Self<String>, Regexp, String, &gsub_ex>(context->string_class, "gsub!");
		method<Self<String>, &empty>(context->string_class, "empty?");
		
		method<Self<String>, Value, Optional<Value>, &rb_get>(context->string_class, "[]");
		
		method<Self<String>, &length>(context->string_class, "length");
		method<Self<String>, &length>(context->string_class, "size");
		method<Self<String>, &ord>(context->string_class, "ord");

		method<Self<String>, &downcase>(context->string_class, "downcase");
		method<Self<String>, &downcase_self>(context->string_class, "downcase!");
		method<Self<String>, &upcase>(context->string_class, "upcase");
		method<Self<String>, &upcase_self>(context->string_class, "upcase!");
		
		method<Self<String>, Arg::UInt, Optional<String>, &ljust>(context->string_class, "ljust");
		method<Self<String>, Arg::UInt, Optional<String>, &rjust>(context->string_class, "rjust");
		method<Self<String>, Optional<Value>, &split>(context->string_class, "split");
		method<Self<String>, Arg::Block, &rb_each_char>(context->string_class, "each_char");
		method<Self<String>, &inspect>(context->string_class, "inspect");
		method<Self<String>, &to_sym>(context->string_class, "to_sym");
		method<Self<String>, String, &String::compare>(context->string_class, "<=>");
		method<Self<String>, Value, &equal>(context->string_class, "==");
		method<Self<String>, Value, &equal>(context->string_class, "===");
		method<Self<String>, String, &concat>(context->string_class, "concat");
		method<Self<String>, String, &concat>(context->string_class, "<<");
		method<Self<String>, String, &add>(context->string_class, "+");
		method<Self<String>, String, String, &tr>(context->string_class, "tr");
	}
};

