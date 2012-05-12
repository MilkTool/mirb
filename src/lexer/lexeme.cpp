#include "lexer.hpp"
#include "../symbol-pool.hpp"

namespace Mirb
{
	std::string Lexeme::names[TYPES] = {
		// values
		"string",
		"array",
		"regular expression",
		"command",
		"heredoc",
		"integer",
		"octal",
		"real",
		"hex",
		"instance variable",
		"class variable",
		"global variable",
		"symbol",
		"identifier",
		"extended identifier",
		
		"None",
		
		"=>",
		"**",
		
		// order contiguous binary operators
		
		"*",
		"/",
		"%",
		
		"+",
		"-",
		
		"<<",
		">>",
		
		"&",
		
		"^",
		"|",
		
		"&&",
		
		"||",
		
		">",
		">=",
		"<",
		"<=",
		
		"<=>",
		"==",
		"===",
		"!=",
		"=~",
		"!~",
		
		
		// keep the assigns in the same order as the operators, insert dummy value if needed
		
		"**=",
		
		"*=",
		"/=",
		"%=",
		
		"+=",
		"-=",
		
		"<<=",
		">>=",
		
		"&=",
		
		"^=",
		"|=",
		
		"&&=",
		
		"||=",
		
		
		"~",
		"!",
		"+@",
		"-@",
		"=",
		"?",
		".",
		"..",
		"...",
		",",
		":",
		"::",
		";",
		"(",
		")",
		"[",
		"]",
		"{",
		"}",
		"newline",
		"end of file",
		
		// keywords
		"if",
		"unless",
		"else",
		"elsif",
		"then",
		"while",
		"until",
		"when",
		"case",
		"begin",
		"ensure",
		"rescue",
		"class",
		"module",
		"def",
		"alias",
		"defined?",
		"self",
		"do",
		"yield",
		"return",
		"break",
		"next",
		"redo",
		"super",
		"true",
		"false",
		"nil",
		"not",
		"and",
		"or",
		"__FILE__",
		"end",
	};

	Lexeme::Lexeme(const Lexeme &lexeme) :
		SourceLoc(lexeme),
		lexer(lexeme.lexer),
		whitespace(lexeme.whitespace),
		allow_keywords(lexeme.allow_keywords),
		error(lexeme.error),
		type(lexeme.type),
		prev(lexeme.prev),
		curlies(lexeme.curlies),
		symbol(lexeme.symbol)
	{
	}
	
	Lexeme& Lexeme::operator=(const Lexeme& other)
	{
		if(this == &other)
			return *this;
		
		SourceLoc::operator=(other);
		
		whitespace = other.whitespace;
		allow_keywords = other.allow_keywords;
		error = other.error;
		type = other.type;
		prev = other.prev;
		curlies = other.curlies;
		symbol = other.symbol;
		data = other.data;
		
		return *this;
	}

	void Lexeme::prev_set(SourceLoc *range)
	{
		range->stop = prev;
	}
	
	std::string Lexeme::describe(SourceLoc *range, Type type)
	{
		std::string result;
		
		if((type >= values_end && type < keyword_start))
			result = "'" + names[type] + "'";
		else if(type >= keyword_start && type <= keyword_end)
			result = "'" + names[type] + "' (keyword)";
		else if(type == IDENT || type == EXT_IDENT)
			result = "'" + range->string() + "' (" + names[type] + ")";
		else
			result = range->string() + " (" + names[type] + ")";
		
		return result;
	}
	
	std::string Lexeme::describe()
	{
		return describe(this, this->type);
	}

	std::string Lexeme::describe_type(Type type)
	{
		std::string result;
		
		if((type >= values_end && type < keyword_start))
			result = "'" + names[type] + "'";
		else if(type >= keyword_start && type <= keyword_end)
			result = "'" + names[type] + "' (keyword)";
		else
			result = names[type];
		
		return result;
	}
	
	SourceLoc &Lexeme::get_prev()
	{
		SourceLoc &result = *new (lexer.memory_pool) SourceLoc;
		result.start = prev;
		result.stop = prev + 1;
		result.line = line;

		const char_t *c = start;

		while(c != prev)
		{
			switch(*c)
			{
				case '\n':
					if(c - 1 != prev && c[-1] == '\r')
						c--;
				
				case '\r':
					result.line--;

				default:
					c--;
			}
		}

		const char_t *input = lexer.input_str;

		switch(*c)
		{
			case '\n':
				c--;
				if(c != input && *c == '\r')
					c--;
				break;

			case '\r':
				c--;
				break;
			
			default:
				break;
		}

		while(c != input)
		{
			switch(*c)
			{
				case '\n':
				case '\r':
					c++;
					goto end;
				
				default:
					c--;
			}
		}

		end:

		result.line_start = c;

		return result;
	}
};
