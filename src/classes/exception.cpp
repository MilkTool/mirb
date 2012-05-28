#include "exception.hpp"
#include "symbol.hpp"
#include "string.hpp"
#include "array.hpp"
#include "../runtime.hpp"
#include "../collector.hpp"
#include "class.hpp"
#include "../block.hpp"
#include "../document.hpp"
#include "../compiler.hpp"
#include "../generic/source-loc.hpp"
#include "../platform/platform.hpp"
#include "../vm.hpp"

namespace Mirb
{
	StackFrame::StackFrame(Frame *frame) :
		Value::Header(Value::InternalStackFrame),
		code(frame->code),
		obj(frame->obj),
		name(frame->name),
		scope(frame->scope),
		block(frame->block),
		ip(frame->ip),
		vars(frame->vars != nullptr)
	{
		args = Tuple<>::allocate(frame->argc);

		for(size_t i = 0; i < frame->argc; ++i)
			(*args)[i] = frame->argv[i];
	}
	
	void StackFrame::print(StackFrame *self)
	{
		OnStack<1> os(self);

		Platform::color<Platform::Gray>("  in ");

		Module *module = self->scope->first();

		if(Value::type(module) == Value::IClass)
			module = module->original_module;

		OnStack<1> os3(module);

		Platform::color<Platform::Gray>(Mirb::inspect(module));

		value_t class_of_obj = class_of(self->obj);

		if(class_of_obj != module)
		{
			Platform::color<Platform::Gray>("(");
			Platform::color<Platform::Gray>(Mirb::inspect(class_of_obj));
			Platform::color<Platform::Gray>(")");
		}

		Platform::color<Platform::Gray>("#");

		std::cerr << self->name->get_string()  << "(";

		for(size_t i = 0; i < self->args->entries; ++i)
		{
			std::cerr << inspect_object((*self->args)[i]);
			if(i < self->args->entries - 1)
				std::cerr << ", ";
		}
			
		std::cerr <<  ")";

		if(self->code->executor == &evaluate_block || self->code->executor == &Compiler::deferred_block)
		{
			SourceLoc *range;

			if(self->vars)
				range = self->code->source_location.get((size_t)(self->ip - self->code->opcodes));
			else
				range = self->code->range;

			if(range)
			{
				CharArray prefix = self->code->document->name + "[" + CharArray::uint(range->line + 1) + "]: ";
				
				Platform::color<Platform::Bold>("\n" + prefix);

				std::cerr << range->get_line() << "\n";
				
				Platform::color<Platform::Green>(CharArray(" ") * prefix.size() + range->indicator());
			}
			else
				Platform::color<Platform::Bold>("\n" + self->code->document->name + "[?]: Unknown");
		}
	}
	
	CharArray StackFrame::inspect_plain_implementation(StackFrame *self)
	{
		CharArray result;

		if(self->code->executor == &evaluate_block)
		{
			SourceLoc *range;

			if(self->vars)
				range = self->code->source_location.get((size_t)(self->ip - self->code->opcodes));
			else
				range = self->code->range;

			if(range)
			{
				result += self->code->document->name + ":" + CharArray::uint(range->line + 1) + ":";
			}
			else
				result += self->code->document->name + ":unknown:";
		}
		else
				result += "mirb:native:";

		result += "in `" + self->name->string + "'";

		return result;
	}
	
	CharArray StackFrame::inspect_implementation(StackFrame *self)
	{
		OnStack<1> os(self);

		CharArray result = "  in ";

		OnStackString<1> os2(result);
		
		Module *module = self->scope->first();

		if(Value::type(module) == Value::IClass)
			module = module->original_module;

		OnStack<1> os3(module);

		result += Mirb::inspect(module);

		value_t class_of_obj = class_of(self->obj);

		if(class_of_obj != module)
		{
			result += "(";
			result += Mirb::inspect(class_of_obj) + ")";
		}

		result += "#" + self->name->string  + "(";

		for(size_t i = 0; i < self->args->entries; ++i)
		{
			result += Mirb::inspect((*self->args)[i]);
			if(i < self->args->entries - 1)
				result += ", ";
		}
			
		result += ")";

		if(self->code->executor == &evaluate_block)
		{
			SourceLoc *range;

			if(self->vars)
				range = self->code->source_location.get((size_t)(self->ip - self->code->opcodes));
			else
				range = self->code->range;

			if(range)
			{
				CharArray prefix = self->code->document->name + "[" + CharArray::uint(range->line + 1) + "]: ";
				result += "\n" + prefix + range->get_line() + "\n" +  CharArray(" ") * prefix.size() + range->indicator();
			}
			else
				result += "\n" + self->code->document->name + "[?]: Unknown";
		}

		return result;
	}
	
	CharArray StackFrame::inspect()
	{
		return inspect_implementation(this);
	}
	
	CharArray StackFrame::inspect_plain()
	{
		return inspect_plain_implementation(this);
	}
	
	Array *StackFrame::get_plain_backtrace(Tuple<StackFrame> *backtrace)
	{
		auto result = Array::allocate();

		if(!backtrace)
			return result;

		OnStack<2> os(backtrace, result);
		
		for(size_t i = 0; i < backtrace->entries; ++i)
		{
			result->vector.push((*backtrace)[i]->inspect_plain().to_string());
		}

		return result;
	}
	
	String *StackFrame::get_backtrace(Tuple<StackFrame> *backtrace)
	{
		CharArray result;

		if(!backtrace)
			return result.to_string();

		OnStack<1> os1(backtrace);
		OnStackString<1> os2(result);
		
		for(size_t i = 0; i < backtrace->entries; ++i)
		{
			if(i != 0)
				result += "\n";

			result += (*backtrace)[i]->inspect();
		}

		return result.to_string();
	}
	
	void StackFrame::print_backtrace(Tuple<StackFrame> *backtrace)
	{
		if(!backtrace)
			return;
		
		OnStack<1> os1(backtrace);
		
		for(size_t i = 0; i < backtrace->entries; ++i)
		{
			if(i != 0)
				std::cerr << "\n";

			print((*backtrace)[i]);
		}
	}
	
	value_t Exception::allocate(Class *instance_of)
	{
		return Collector::allocate<Exception>(Value::Exception, instance_of, nullptr, nullptr);
	}
	
	value_t Exception::to_s(Exception *self)
	{
		if(self->message)
			return self->message;
		else
			return value_nil;
	}

	value_t Exception::awesome_backtrace(Exception *self)
	{
		if(self->backtrace)
			return StackFrame::get_backtrace(self->backtrace);
		else
			return value_nil;
	}
	
	value_t Exception::rb_backtrace(Exception *self)
	{
		if(self->backtrace)
			return StackFrame::get_plain_backtrace(self->backtrace);
		else
			return value_nil;
	}

	value_t Exception::rb_initialize(Exception *self, String *message)
	{
		self->message = message;

		return self;
	}

	void Exception::initialize()
	{
		context->exception_class = define_class("Exception", context->object_class);

		singleton_method<Arg::Self<Arg::Class<Class>>, &allocate>(context->exception_class, "allocate");

		method<Arg::Self<Arg::Class<Exception>>, Arg::Class<String>, &rb_initialize>(context->exception_class, "initialize");
		method<Arg::Self<Arg::Class<Exception>>, &awesome_backtrace>(context->exception_class, "awesome_backtrace");
		method<Arg::Self<Arg::Class<Exception>>, &rb_backtrace>(context->exception_class, "backtrace");
		method<Arg::Self<Arg::Class<Exception>>, &to_s>(context->exception_class, "message");
		method<Arg::Self<Arg::Class<Exception>>, &to_s>(context->exception_class, "to_s");
	}
};

