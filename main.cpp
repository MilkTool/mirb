#include "globals.hpp"
#include "runtime/code_heap.hpp"
#include "runtime/classes.hpp"
#include "runtime/classes/symbol.hpp"
#include "runtime/classes/string.hpp"

#include <iostream>
#include "src/compiler.hpp"
#include "src/block.hpp"
#include "src/mem_stream.hpp"
#include "src/arch/codegen.hpp"
#include "src/codegen/bytecode.hpp"

#ifdef DEBUG
	#include "src/tree/printer.hpp"
#endif

using namespace Mirb;

int main()
{
	#ifndef NO_GC
		GC_INIT();
	#endif
	
	rt_create();
	
	while(1)
	{
		std::string line;
		
		std::getline(std::cin, line);
		
		Compiler compiler;
		
		compiler.filename = "Input";
		compiler.load((const char_t *)line.c_str(), line.length());
		
		if(compiler.parser.lexeme() == Lexeme::END && compiler.messages.empty())
			break;
		
		Tree::Fragment fragment(0, Tree::Chunk::main_size);
		Tree::Scope *scope = compiler.parser.parse_main(&fragment);
		
		if(!compiler.messages.empty())
		{
			for(auto i = compiler.messages.begin(); i != compiler.messages.end(); ++i)
				std::cout << i().format() << "\n";
				
			continue;
		}
	
		#ifdef DEBUG
			DebugPrinter printer;
			
			std::cout << "Parsing done.\n-----\n";
			std::cout << printer.print_node(scope->group);
			std::cout << "\n-----\n";
		#endif
		
		CodeGen::ByteCodeGenerator generator(compiler.memory_pool);
		
		CodeGen::Block *block = generator.to_bytecode(scope);

		size_t block_size = CodeGen::NativeMeasurer::measure(block);

		void *block_code = rt_code_heap_alloc(block_size);

		MemStream stream(block_code, block_size);

		CodeGen::NativeGenerator native_generator(stream, compiler.memory_pool);

		native_generator.generate(block);
		
		//__asm__("int3\n"); // Make debugging life easier
		
		rt_value result = block->final->compiled(RT_NIL, rt_class_of(rt_main), rt_main, RT_NIL, 0, 0);
		
		printf("=> "); rt_print(result); printf("\n");
		
		block = 0; // Make sure block stays on stack */
	}
	
	std::cout << "Exiting gracefully...";
	
	rt_destroy();
	
	return 0;
}
