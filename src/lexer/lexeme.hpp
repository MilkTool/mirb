#pragma once
#include <Prelude/Vector.hpp>
#include "../common.hpp"
#include "../symbol-pool.hpp"
#include "../generic/source-loc.hpp"

namespace Mirb
{
	class Lexer;
	
	struct InterpolateData;
	struct InterpolateState;
	struct DataEntry;

	namespace Tree
	{
		struct HeredocNode;
	};

	class Lexeme:
		public SourceLoc
	{
		public:
			enum Type
			{
				// values
				STRING,
				ARRAY,
				REGEXP,
				COMMAND,
				HEREDOC,
				INTEGER,
				OCTAL,
				BINARY,
				REAL,
				HEX,
				IVAR,
				CVAR,
				GLOBAL,
				SYMBOL,
				IDENT,
				EXT_IDENT,
				
				NONE,
				
				ASSOC,
				BACKTICK,
				BLOCK_POINT,
				POWER,
				
				// order contiguous binary operators
				
				MUL,
				DIV,
				MOD,
				
				ADD,
				SUB,
				
				LEFT_SHIFT,
				RIGHT_SHIFT,
				
				AMPERSAND, // BITWISE_AND
				
				BITWISE_XOR,
				BITWISE_OR, // block parameter start/end
				
				LOGICAL_AND,
				
				LOGICAL_OR,
				
				GREATER,
				GREATER_OR_EQUAL,
				LESS,
				LESS_OR_EQUAL,
				
				COMPARE,
				EQUALITY,
				CASE_EQUALITY,
				NO_EQUALITY,
				MATCHES,
				NOT_MATCHES,
				
								
				// keep the assigns in the same order as the operators
				
				ASSIGN_POWER,
				
				ASSIGN_MUL,
				ASSIGN_DIV,
				ASSIGN_MOD,
				
				ASSIGN_ADD,
				ASSIGN_SUB,
				
				ASSIGN_LEFT_SHIFT,
				ASSIGN_RIGHT_SHIFT,
				
				ASSIGN_BITWISE_AND,
				
				ASSIGN_BITWISE_XOR,
				ASSIGN_BITWISE_OR,
				
				ASSIGN_LOGICAL_AND,
				
				ASSIGN_LOGICAL_OR,
				
				
				BITWISE_NOT,
				LOGICAL_NOT,
				
				// must be in the same order as ADD and SUB
				UNARY_ADD,
				UNARY_SUB,
				
				ASSIGN,
				QUESTION,
				DOT,
				RANGE_INCL,
				RANGE_EXCL,
				COMMA,
				COLON,
				SCOPE,
				SEMICOLON,
				PARENT_OPEN,
				PARENT_CLOSE,
				SQUARE_OPEN,
				SQUARE_CLOSE,
				CURLY_OPEN,
				CURLY_CLOSE,
				LINE,
				END,
				
				// Keywords
				KW_IF,
				KW_UNLESS,
				KW_ELSE,
				KW_ELSIF,
				KW_THEN,
				KW_WHILE,
				KW_UNTIL,
				KW_FOR,
				KW_IN,
				KW_WHEN,
				KW_CASE,
				KW_BEGIN,
				KW_ENSURE,
				KW_RESCUE,
				KW_CLASS,
				KW_MODULE,
				KW_DEF,
				KW_ALIAS,
				KW_UNDEF,
				KW_DEFINED,
				KW_SELF,
				KW_DO,
				KW_YIELD,
				KW_RETURN,
				KW_BREAK,
				KW_NEXT,
				KW_REDO,
				KW_SUPER,
				KW_TRUE,
				KW_FALSE,
				KW_NIL,
				KW_NOT,
				KW_AND,
				KW_OR,
				KW_SPECIAL_FILE,
				KW_SPECIAL_LINE,
				KW_SPECIAL_BEGIN,
				KW_SPECIAL_END,
				KW_END,
				TYPES
			};
			
			static inline Type operator_to_assign(Type op)
			{
				return (Type)((size_t)op + ((size_t)ASSIGN_POWER - (size_t)POWER));
			}
			
			static inline Type assign_to_operator(Type op)
			{
				return (Type)((size_t)op - ((size_t)ASSIGN_POWER - (size_t)POWER));
			}
			
			static inline Type operator_to_unary(Type op)
			{
				return (Type)((size_t)op + ((size_t)UNARY_ADD - (size_t)ADD));
			}
			
			static const Type precedence_operators_start = MUL;
			static const Type precedence_operators_end = NOT_MATCHES;
			static const Type keyword_start = KW_IF;
			static const Type keyword_end = KW_END;
			static const Type values_end = NONE;

			Lexeme(MemoryPool memory_pool) : curlies(memory_pool) {}
			
			const char_t *current_line_start;
			size_t current_line;

			bool whitespace;
			bool allow_keywords;
			bool error;
			
			Type type;
			const char_t *prev;
			
			Vector<InterpolateState *, MemoryPool> curlies;
			
			union
			{
				Symbol *symbol;
				InterpolateData *data;
				DataEntry *number;
				Tree::HeredocNode *heredoc;
			};

			void prev_set(SourceLoc *range);
			
			static std::string names[TYPES];
			
			CharArray describe();
			
			static CharArray describe(SourceLoc *range, Type type);
			static CharArray describe_type(Type type);
	};
	
	struct Heredoc;

	struct InterpolateState
	{
		SourceLoc *start;
		Lexeme::Type type;
		char_t opener;
		char_t terminator;
		Heredoc *heredoc;
		size_t nested;

		InterpolateState() : start(0), heredoc(0), nested(0) {}
	};
	
	struct DataEntry
	{
		const char_t *data;
		size_t length;

		DataEntry() : data(0), length(0) {}

		template<typename A> DataEntry copy(typename A::Reference ref = A::default_reference) const
		{
			DataEntry result;

			result.data = (const char_t *)A(ref).allocate(length);
			std::memcpy((void *)result.data, data, length);
			result.length = length;

			return result;
		}
			
		CharArray get() const
		{
			return CharArray(data, length);
		}
			
		template<typename A> void add(const CharArray &add, typename A::Reference ref = A::default_reference)
		{
			set<A>(get() + add, ref);
		};

		template<typename A> void set(const CharArray &other, typename A::Reference ref = A::default_reference)
		{
			length = other.size();

			data = (const char_t *)A(ref).allocate(length);

			std::memcpy((void *)data, other.raw(), length);
		};
	};
	
	struct InterpolateData
	{
		struct AdvancedEntry:
			public DataEntry
		{
			SourceLoc range;
			Lexeme::Type type;
			Symbol *symbol;
		};
	
		Vector<AdvancedEntry *, MemoryPool> entries;

		enum Type
		{
			Plain,
			Starting,
			Continuing,
			Ending
		};

		DataEntry tail;
		Type type;

		bool beginning()
		{
			return type == Plain || type == Starting;
		}

		InterpolateData(MemoryPool memory_pool) : entries(memory_pool), type(Plain) {}
	};

};
