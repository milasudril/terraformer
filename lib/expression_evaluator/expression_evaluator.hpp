#ifndef TERRAFORMER_EXPRESSION_EVALUATOR_HPP
#define TERRAFORMER_EXPRESSION_EVALUATOR_HPP

#include "lib/common/input_error.hpp"

#include <string_view>
#include <stack>
#include <string>
#include <vector>
#include <span>

namespace terraformer::expression_evaluator
{
	constexpr bool is_whitespace(char ch_in)
	{ return ch_in>='\0' && ch_in <= ' '; }

	constexpr bool is_delimiter(char ch_in)
	{ return ch_in == ',' || ch_in == '(' || ch_in == ')'; }

	enum class parser_state{
		init,
		read_comand_name,
		before_list_item,
		read_list_item,
		after_list_item,
		after_command,
		after_command_name,
	};

	template<class ArgumentType>
	struct parse_result
	{
		ArgumentType result;
		std::string_view::iterator expression_end;
	};

	template<class ContextEvaluator>
	[[nodiscard]] auto parse(std::string_view expression, ContextEvaluator&& evaluator)
	{
		auto ptr = std::begin(expression);
		auto current_state = parser_state::init;

		std::string buffer;
		using context_type = decltype(evaluator.make_context(std::declval<std::string>()));
		std::stack<context_type> contexts;
		context_type* current_context = nullptr;
		while(ptr != std::end(expression))
		{
			auto ch_in = *ptr;
			++ptr;
			switch(current_state)
			{
				case parser_state::init:
					if(is_delimiter(ch_in))
					{ throw input_error{"Empty command name"}; }

					if(!is_whitespace(ch_in))
					{
						buffer += ch_in;
						current_state = parser_state::read_comand_name;
					}
					break;

				case parser_state::read_comand_name:
					switch(ch_in)
					{
						case '(':
							contexts.push(evaluator.make_context(std::move(buffer)));
							buffer.clear();
							current_context = &contexts.top();
							current_state = parser_state::before_list_item;
							break;

						case ',':
							throw input_error{"A command name may not contain `,`"};

						case ')':
							throw input_error{"A command name may not contain `)`"};

						default:
							if(is_whitespace(ch_in))
							{ current_state = parser_state::after_command_name; }
							else
							{ buffer += ch_in; }
					}
					break;

				case parser_state::before_list_item:
					switch(ch_in)
					{
						case '(':
							throw input_error{"A list item may not contain `(`"};

						case ',':
							throw input_error{"A list item may not contain `,`"};

						case ')':
						{
							auto res = evaluator.evaluate(*current_context);
							contexts.pop();
							if(contexts.empty())
							{ return parse_result{std::move(res), ptr}; }
							current_context = &contexts.top();
							current_context->args.push_back(std::move(res));
							current_state = parser_state::after_command;
							break;
						}

						default:
							if(!is_whitespace(ch_in))
							{
								buffer += ch_in;
								current_state = parser_state::read_list_item;
							}
					}
					break;

				case parser_state::read_list_item:
					switch(ch_in)
					{
						case '(':
							contexts.push(evaluator.make_context(std::move(buffer)));
							buffer.clear();
							current_context = &contexts.top();
							current_state = parser_state::before_list_item;
							break;

						case ',':
							current_context->args.push_back(evaluator.make_argument(std::move(buffer)));
							buffer.clear();
							current_state = parser_state::before_list_item;
							break;

						case ')':
						{
							if(!buffer.empty())
							{
								current_context->args.push_back(evaluator.make_argument(std::move(buffer)));
								buffer.clear();
							}
							auto res = evaluator.evaluate(*current_context);
							contexts.pop();
							if(contexts.empty())
							{ return parse_result{res, ptr}; }
							current_state = parser_state::after_command;
							current_context = &contexts.top();
							current_context->args.push_back(res);
							break;
						}

						default:
							if(is_whitespace(ch_in))
							{
								if(!buffer.empty())
								{ current_state = parser_state::after_list_item; }
							}
							else
							{ buffer += ch_in; }
					}
					break;

				case parser_state::after_list_item:
					switch(ch_in)
					{
						case '(':
							contexts.push(evaluator.make_context(std::move(buffer)));
							buffer.clear();
							current_context = &contexts.top();
							current_state = parser_state::read_list_item;
							break;

						case ',':
							current_context->args.push_back(evaluator.make_argument(std::move(buffer)));
							buffer.clear();
							current_state = parser_state::read_list_item;
							break;

						case ')':
						{
							if(!buffer.empty())
							{
								current_context->args.push_back(evaluator.make_argument(std::move(buffer)));
								buffer.clear();
							}
							auto res = evaluator.evaluate(*current_context);
							contexts.pop();
							if(contexts.empty())
							{ return parse_result{res, ptr}; }
							current_state = parser_state::after_command;
							current_context = &contexts.top();
							current_context->args.push_back(res);
							break;
						}

						default:
							if(!is_whitespace(ch_in))
							{ throw input_error{"Unexpected character at end of list item"}; }
					}
					break;

				case parser_state::after_command:
					switch(ch_in)
					{
						case ',':
							current_state = parser_state::read_list_item;
							break;

						case ')':
						{
							auto res = evaluator.evaluate(*current_context);
							contexts.pop();
							if(contexts.empty())
							{ return parse_result{res, ptr}; }
							current_context = &contexts.top();
							current_context->args.push_back(res);
							break;
						}

						default:
							if(!is_whitespace(ch_in))
							{ throw input_error{"Unexpected character at end of command"}; }
					}
					break;

				case parser_state::after_command_name:
					switch(ch_in)
					{
						case '(':
							contexts.push(evaluator.make_context(std::move(buffer)));
							buffer.clear();
							current_context = &contexts.top();
							current_state = parser_state::before_list_item;
							break;

						default:
							if(!is_whitespace(ch_in))
							{ throw input_error{"Unexpected character after command name"}; }
					}
					break;
			}
		}

		if(!contexts.empty())
		{ throw input_error{"Unterminated command"}; }

		return parse_result{evaluator.make_argument(std::move(buffer)), ptr};
	}
}

#endif