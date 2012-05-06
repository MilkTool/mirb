#pragma once
#include "../common.hpp"
#include "../lexer/lexer.hpp"
#include "../tree/nodes.hpp"
#include "../tree/tree.hpp"
#include "../message.hpp"
#include <Prelude/List.hpp>

namespace Mirb
{
	class Document;

	class Parser
	{
		public:
			Parser(SymbolPool &symbol_pool, MemoryPool memory_pool, Document *document);
			~Parser();
			
			Lexer lexer;

			List<Message> messages;
			Document &document;

			void add_message(Message *message, const Range &range);

			MemoryPool memory_pool;
			
			typedef List<Tree::VoidNode, Tree::VoidNode, &Tree::VoidNode::void_entry> VoidList;

			bool close_pair(const std::string &name, const Range &range, Lexeme::Type lexeme, bool skip = true);

			VoidList *void_list;

			class VoidTrapper
			{
				private:
					Parser *parser;
					VoidList *prev;
				public:
					VoidList list;

					VoidTrapper(Parser *parser) : parser(parser), prev(parser->void_list)
					{
						parser->void_list = &list;
					}
				
					void release()
					{
						if(prev)
						{
							parser->void_list = prev;
							prev = nullptr;
						}
					}
				
					void raise()
					{
						for(auto node: list)
							parser->raise_void(node);

						list.clear();
					}

					~VoidTrapper()
					{
						release();
						raise();
					}
			};

			Tree::VoidNode *raise_void(Tree::VoidNode *node)
			{
				void_list->append(node);
				return node;
			}
			
			void load();
			
			void report(const Range &range, std::string text, Message::Severity severity = Message::MESSAGE_ERROR);
			
			Tree::Fragment fragment;
			Tree::Scope *scope;
			
			void unexpected(bool skip = true);
			void expected(Lexeme::Type what, bool skip = false);
			
			void error(std::string text);
			
			template<typename F> void allocate_scope(Tree::Scope::Type type, F func)
			{
				Tree::Scope *current_scope = scope;
				Tree::Fragment current_fragment = fragment;

				if(type == Tree::Scope::Closure || type == Tree::Scope::Method)
					fragment = *new Tree::FragmentBase(Tree::Chunk::block_size);
		
				scope = Collector::allocate_pinned<Tree::Scope>(&document, fragment, scope, type);
				
				VoidTrapper trapper(this);

				func();
				
				if(type != Tree::Scope::Closure)
				{
					for(auto node: trapper.list)
						if(node->type() != Tree::Node::Return)
							report(*node->range, Tree::SimpleNode::names[node->type()] + " outside of block.");
				}
				else
				{
					for(auto node: trapper.list)
						if(node->type() == Tree::Node::Break)
						{
							scope->break_id = scope->parent->break_targets++;
							break;
						}
				}

				trapper.list.clear();

				scope = current_scope;
				fragment = current_fragment;
			}
			
			static bool is_constant(Symbol *symbol);
						
			static size_t operator_precedences[];

			bool is_precedence_operator(Lexeme::Type op);
			Tree::Node *parse_precedence_operator();
			Tree::Node *parse_precedence_operator(Tree::Node *left, size_t min_precedence);
			
			void process_string_entries(Tree::InterpolatedNode *root, StringData::Entry &tail);
			
			template<typename F> void typecheck(Tree::Node *&result, F func)
			{
				if(result->single())
				{
					result = func(result);
				}
				else
				{
					auto node = (Tree::MultipleExpressionsNode *)result;

					if(!node->extended)
						node->extended = capture();

					func(nullptr);

					lexer.lexeme.prev_set(node->extended);
				}
			};
			
			Tree::Node *typecheck(Tree::Node *result);

			// expressions
			Tree::Node *parse_variable(Symbol *symbol, Range *range);
			bool is_assignment_op();
			bool is_equality_op();
			bool is_sep();
			void skip_seps();
			Tree::Node *parse_hash();
			Tree::StringNode *parse_string(Value::Type type);
			Tree::Node *parse_power();
			Tree::Node *parse_splat_expression();
			void process_lhs(Tree::Node *&lhs, const Range &range);
			void process_multiple_lhs(Tree::MultipleExpressionsNode *node);
			Tree::Node *parse_multiple_expressions(bool allow_multiples);
			Tree::Node *parse_assignment(bool allow_multiples);
			Tree::Node *build_assignment(Tree::Node *left, bool allow_multiples);
			Tree::Node *process_assignment(Tree::Node *input, bool allow_multiples);
			Tree::Node *parse_array();
			Tree::Node *parse_unary();
			Tree::Node *parse_boolean_unary();
			Tree::Node *parse_factor();
			Tree::Node *parse_boolean();
			
			Tree::Node *parse_operator_expression(bool allow_multiples = true)
			{
				return typecheck(parse_assignment(allow_multiples));
			}
			
			Tree::Node *parse_expression()
			{
				return typecheck(parse_boolean());
			}
			
			Tree::Node *parse_group()
			{
				return typecheck(parse_statements());
			}

			Tree::Node *parse_statement()
			{
				return parse_tailing_loop();
			}
			
			Tree::Node *parse_statements();
			
			void parse_sep();

			Tree::Scope *parse_main();
			
			// control flow
			bool is_jump_argument();
			Tree::Node *parse_if_tail();
			void parse_then_sep();
			Tree::Node *parse_if();
			Tree::Node *parse_unless();
			Tree::Node *parse_ternary_if();
			Tree::Node *parse_conditional();
			Tree::Node *parse_tailing_loop();
			Tree::Node *parse_loop();
			Tree::Node *process_loop(Tree::LoopNode *loop, VoidTrapper &trapper);
			Tree::Node *parse_case();
			Tree::Node *parse_begin();
			Tree::Node *parse_exception_handlers(Tree::Node *block, VoidTrapper &trapper);
			Tree::Node *parse_return();
			Tree::Node *parse_break();
			Tree::Node *parse_next();
			Tree::Node *parse_redo();
			
			// calls
			bool require_ident();
			bool has_arguments();
			bool is_lookup();
			Tree::Node *parse_lookup(Tree::Node *child);
			Tree::BlockNode *parse_block(bool allowed);
			void parse_arguments(Tree::InvokeNode *node);
			void parse_arguments(Tree::InvokeNode *node, bool *parenthesis);
			Tree::Node *alloc_call_node(Tree::Node *object, Symbol *symbol, Range *range, bool has_args, bool can_be_var = false);
			Tree::Node *parse_call(Symbol *symbol, Tree::Node *child, Range *range, bool default_var);
			Tree::Node *parse_lookup_chain();
			Tree::Node *parse_yield();
			Tree::Node *parse_super();
			
			// structures
			bool is_parameter();
			void parse_parameters();
			Tree::Node *parse_class();
			Tree::Node *parse_module();
			Tree::Node *parse_method();
			void parse_module_name(Tree::ModuleNode &node);
			
			Lexeme::Type lexeme()
			{
				return lexer.lexeme.type;
			}
			
		private:
			bool match(Lexeme::Type what)
			{
				if(lexeme() == what)
				{
					lexer.step();
					return true;
				}
				else
				{
					expected(what);
					return false;
				}
			}
			
			bool require(Lexeme::Type what)
			{
				if(lexeme() == what)
				{
					return true;
				}
				else
				{
					expected(what);
					return false;
				}
			}
			
			bool matches(Lexeme::Type what)
			{
				if(lexeme() == what)
				{
					lexer.step();
					return true;
				}
				else
					return false;
			}
			
			bool expect(Lexeme::Type what)
			{
				if(lexeme() == what)
				{
					return true;
				}
				else
				{
					expected(what);
					return false;
				}
			}
			
			void skip_lines()
			{
				while(matches(Lexeme::LINE));
			}

			Range *capture();

			Range *parse_method_name(Symbol *&symbol);

			bool is_expression()
			{
				switch(lexeme())
				{
					case Lexeme::IDENT:
					case Lexeme::EXT_IDENT:
					case Lexeme::UNARY_ADD:
					case Lexeme::UNARY_SUB:
					case Lexeme::IVAR:
					case Lexeme::GLOBAL:
					case Lexeme::CVAR:
					case Lexeme::SYMBOL:
					case Lexeme::ADD:
					case Lexeme::SUB:
					case Lexeme::MUL:
					case Lexeme::INTEGER:
					case Lexeme::KW_IF:
					case Lexeme::KW_UNLESS:
					case Lexeme::KW_CASE:
					case Lexeme::KW_BEGIN:
					case Lexeme::KW_CLASS:
					case Lexeme::KW_MODULE:
					case Lexeme::KW_NOT:
					case Lexeme::LOGICAL_NOT:
					case Lexeme::KW_DEF:
					case Lexeme::KW_SELF:
					case Lexeme::KW_TRUE:
					case Lexeme::KW_FALSE:
					case Lexeme::KW_SPECIAL_FILE:
					case Lexeme::KW_NIL:
					case Lexeme::STRING:
					case Lexeme::STRING_START:
					case Lexeme::KW_YIELD:
					case Lexeme::KW_RETURN:
					case Lexeme::KW_BREAK:
					case Lexeme::KW_SUPER:
					case Lexeme::KW_REDO:
					case Lexeme::KW_NEXT:
					case Lexeme::KW_WHILE:
					case Lexeme::KW_UNTIL:
					case Lexeme::PARENT_OPEN:
					case Lexeme::SQUARE_OPEN:
					case Lexeme::CURLY_OPEN:
					case Lexeme::COLON:
					case Lexeme::SCOPE:
					case Lexeme::QUESTION:
						return true;

					default:
						return false;
				}
			}
	};
	
};
