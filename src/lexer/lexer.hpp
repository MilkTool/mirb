#pragma once
#include "../common.hpp"
#include <Prelude/Map.hpp>
#include "../generic/source-loc.hpp"
#include "../message.hpp"
#include "../tree/tree.hpp"
#include "input.hpp"
#include "lexeme.hpp"

namespace Mirb
{
	class Parser;
	class Symbol;
	class SymbolPool;

	class Keywords
	{
		public:
			Keywords(SymbolPool &pool);
			
			Map<Symbol *, Lexeme::Type> mapping;
	};
	
	struct Heredoc
	{
		DataEntry name;
		char_t type;
		bool remove_ident;
		Tree::HeredocNode *node;
		Tree::Scope *scope;
		SourceLoc range;
	};

	class Lexer
	{
		private:
			Input input;
			
			SymbolPool &symbol_pool;
			Parser &parser;

			static Map<char_t, char_t> delimiter_mapping;
			static void(Lexer::*jump_table[sizeof(char_t) << 8])();

			bool process_null(const char_t *input, bool expected = false);
			void process_comment();

			void restep(bool whitespace = false);
			
			void report_null();

			bool heredoc_terminates(Heredoc *heredoc);
			void regexp_options();
			
			void parse_interpolate_heredoc(Heredoc *heredoc);
			bool parse_escape(CharArray &result, bool capture_newline, bool no_heredoc, bool &newline);
			void parse_interpolate(InterpolateState *state, bool continuing);
			void parse_delimited_data(Lexeme::Type type);
			void parse_simple_string(char_t opener, char_t terminator, bool has_opener);

			void report(const SourceLoc &range, const CharArray &text, Message::Severity severity = Message::MESSAGE_ERROR);
			
			SourceLoc range(const char_t *start, const char_t *stop); 

			template<Lexeme::Type type> void single();
			template<Lexeme::Type type, Lexeme::Type assign_type> void assign();
			template<Lexeme::Type type, Lexeme::Type assign_type, char_t match, Lexeme::Type match_type, Lexeme::Type match_assign> void assign();
			
			template<typename F> void skip_numbers(CharArray &result, F test)
			{
				bool trailing = false;
				const char_t *pos = nullptr;

				while(test())
				{
					result += input++;

					if(input == '_')
					{
						pos = &input;
						input++;
						trailing = true;
						
						if(input == '_')
						{
							const char_t *start = &input;

							while(input == '_')
								input++;
							
							report(range(start, &input), "Only one underscore is allowed between digits in number literals");
						}

					}
					else
						trailing = false;
				}

				if(trailing)
					report(range(pos, pos + 1), "Trailing '_' in number");
			}

			template<Lexeme::Type type, char_t high> void get_number()
			{
				CharArray result;

				skip_numbers(result, [&]() mutable { return input.in('0', high); });
				
				if(input.in('0', '9'))
				{
					number();

					report(lexeme, "Invalid " + Lexeme::describe_type(type) + " number");
				}
				else
				{
					lexeme.number = new (memory_pool) DataEntry;
					lexeme.number->set<MemoryPool>(result, memory_pool);
					lexeme.type = type;
					lexeme.stop = &input;
				}
			}

			bool exponent();
			void eol();
			bool is_white();
			void white();
			void unknown();
			void null();
			void newline();
			void process_newline(bool no_heredoc, bool allow_comment);
			void skip_line();
			void carrige_return();
			
			void assign_equal();
			void colon();
			void compare();
			
			void exclamation();
			
			void comment();
			
			void curly_open();
			void curly_close();

			void sub();
			
			void ivar();
			void global();
			static bool is_char(char_t c);
			static bool is_alpha(char_t c);
			static bool is_ident(char_t c);
			static bool is_start_ident(char_t c);
			void skip_ident();
			
			void zero();
			void number();
			void real();
			void hex();
			
			void ident();
			void string();
			void command();
			void simple_string();
		public:
			static void setup_jump_table();

			Lexer(SymbolPool &symbol_pool, MemoryPool memory_pool, Parser &parser);
			
			MemoryPool memory_pool;
			Keywords keywords;
			
			Lexeme lexeme;
			
			Vector<Heredoc *, MemoryPool> heredocs;

			const char_t *input_str;
			size_t length;
			
			void mod_to_literal();
			void div_to_regexp();
			void left_shift_to_heredoc();
			void question_to_character();
			void expand_to_unary();
			bool whitespace_after();

			void load(const char_t *input, size_t length);
			void step();
			void identify_keywords();
			void done();
	};
};
