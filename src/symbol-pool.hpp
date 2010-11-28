#pragma once
#include "common.hpp"
#include "generic/hash-table.hpp"
#include "gc.hpp"
#include "classes/symbol.hpp"

namespace Mirb
{
	class SymbolPoolFunctions:
		public HashTableFunctions<CharArray &, Symbol *, GC>
	{
		public:
			static bool compare_key_value(CharArray &key, Symbol *value)
			{
				return key == value->string;
			}

			static CharArray &get_key(Symbol *value)
			{
				return value->string;
			}

			static Symbol *get_value_next(Symbol *value)
			{
				return value->next;
			}

			static void set_value_next(Symbol *value, Symbol *next)
			{
				value->next = next;
			}
			
			static size_t hash_key(CharArray &key)
			{
				return key.hash();
			}
			
			static bool valid_key(CharArray &key)
			{
				return true;
			}

			static bool create_value()
			{
				return true;
			}

			static Symbol *create_value(GC::Ref alloc_ref, CharArray &key)
			{
				Symbol *result = new (gc) Symbol(key);

				result->next = 0;

				return result;
			}
	};

	typedef HashTable<CharArray &, Symbol *, SymbolPoolFunctions, GC> SymbolPoolHashTable;

	class SymbolPool:
		public SymbolPoolHashTable
	{
		public:
			SymbolPool() : SymbolPoolHashTable(8) {}
			
			Symbol *get(CharArray &char_array)
			{
				return SymbolPoolHashTable::get(char_array);
			}

			Symbol *get(const char *string)
			{
				CharArray key((const char_t *)string);

				return SymbolPoolHashTable::get(key);
			}

			Symbol *get(const std::string &string)
			{
				CharArray key(string);

				return SymbolPoolHashTable::get(key);
			}

			Symbol *get(const char_t *string, size_t length)
			{
				CharArray key(string, length);

				return SymbolPoolHashTable::get(key);
			}

			Symbol *get(Range &range);

			Symbol *get(const char_t *start, const char_t *stop)
			{
				CharArray key(start, stop - start);

				return SymbolPoolHashTable::get(key);
			}
	};

	extern SymbolPool symbol_pool;
};
