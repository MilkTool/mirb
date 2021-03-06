#pragma once
#include "allocator.hpp"
#include <Prelude/Vector.hpp>
#include <Prelude/LinkedList.hpp>
#include "value-map.hpp"
#include "char-array.hpp"

namespace Mirb
{
	class Frame;
	class Stream;

	class ThreadContext
	{
		public:
			LinkedListEntry<ThreadContext> entry;

			CharArray current_directory;

			Exception *exception;

			template<typename F> void mark(F mark)
			{
				mark(current_directory);

				if(exception)
					mark(exception);
			}

			ThreadContext();
	};

	class Context
	{
		public:
			struct Symbols
			{
				Symbol *classpath;
				Symbol *class_scope;
				Symbol *classname;
				Symbol *attached;
				Symbol *compare;
				Symbol *equal;

				Symbol *terminator;
			};
			
			Class *object_class;
			Class *class_class;
			Class *array_class;
			Class *hash_class;
			Class *string_class;
			Class *regexp_class;
			Class *proc_class;
			Class *symbol_class;
			Class *nil_class;
			Class *true_class;
			Class *false_class;
			Class *module_class;
			Class *method_class;
			Class *dir_class;
			Class *range_class;
			Class *time_class;
			
			Class *io_class;
			Class *file_class;
			
			Class *numeric_class;
			Class *float_class;

			Class *integer_class;
			Class *fixnum_class;
			Class *bignum_class;

			Class *exception_class;
			
			Class *system_exit_class;

			Class *system_stack_error;

			Class *signal_exception;
			Class *interrupt_class;

			Class *standard_error;
			Class *io_error;
			Class *name_error;
			Class *type_error;
			Class *syntax_error;
			Class *argument_error;
			Class *runtime_error;
			Class *local_jump_error;
			Class *load_error;
			Class *system_call_error;
			Class *zero_division_error;
			Class *index_error;

			Class *stop_iteration_class;

			Module *kernel_module;
			Module *comparable_module;
			Module *enumerable_module;
			Module *process_module;
			
			Method *inspect_method;
			
			IO *io_in;
			IO *io_out;
			IO *io_err;

			Tuple<Module> *object_scope;

			Symbols syms;

			Class *class_of_table[literal_count];

			Object *main;

			ValueMap *dummy_map;

			Array *load_paths;
			Array *loaded_files;
			
			Class *terminator;
			
			bool bootstrap;
			bool console;

			Frame *frame;

			Stream *console_input;
			Stream *console_output;
			Stream *console_error;

			ValueMapData globals;
			Vector<value_t, AllocatorBase, Allocator> at_exits;

			Context();
			
			bool accessing(bool = true)
			{
				mirb_runtime_abort("Should not be called");
			}
			
			template<typename F> void mark(F mark)
			{
				auto start = &object_class;

				while(start != &terminator)
				{
					if(*start)
						mark(*start);

					start++;
				}
				
				at_exits.mark(mark);
				globals.mark(mark);
			}

			void setup();
	};
	
	typedef ValueMapManipulator<true, Context, &Context::globals> GlobalAccess;
	
	extern Context *context;
	extern LinkedList<ThreadContext> thread_contexts;
	extern prelude_thread ThreadContext *thread_context;
};
