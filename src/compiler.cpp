#include "compiler.hpp"
#include "symbol-pool.hpp"
#include "block.hpp"
#include "runtime.hpp"
#include "codegen/bytecode.hpp"
#include "classes/exceptions.hpp"

#ifdef MIRB_DEBUG_COMPILER
	#include "codegen/printer.hpp"
#endif

namespace Mirb
{
	Block *Compiler::compile(Tree::Scope *scope, MemoryPool memory_pool)
	{
		Value::assert_valid(scope);

		CodeGen::ByteCodeGenerator generator(memory_pool);
		
		CodeGen::Block *block = generator.to_bytecode(scope);

		#ifdef MIRB_DEBUG_COMPILER
			CodeGen::ByteCodePrinter printer(block);

			std::cout << printer.print() << std::endl;
		#endif
		
		block->finalize();

		return block->final;
	}

	value_t deferred_block(Frame &frame)
	{
		{
			MemoryPool::Base memory_pool;

			Compiler::compile(frame.code->scope, memory_pool);
		}

		return frame.code->executor(frame);
	}

	Block *Compiler::defer(Tree::Scope *scope)
	{
		Block *block = Collector::allocate_pinned<Block>(scope->document);
		
		scope->final = block;
		block->scope = scope;
		block->executor = deferred_block;

		return block;
	}
};
