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
	struct parser_context
	{
		std::string command_name;
		std::vector<ArgumentType> args;
	};

	template<class ArgumentType>
	struct parse_result
	{
		ArgumentType result;
		std::string_view::iterator expression_end;
	};

	template<class ContextEvaluator, class StringConverter>
	[[nodiscard]] auto parse(std::string_view expression, ContextEvaluator&& evaluator, StringConverter&& string_converter)
	{
		auto ptr = std::begin(expression);
		auto current_state = parser_state::init;

		std::string buffer;
		using argument_type = typename std::remove_cvref_t<ContextEvaluator>::argument_type;
		std::stack<parser_context<argument_type>> contexts;
		parser_context<argument_type>* current_context = nullptr;
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
							contexts.push(parser_context{std::move(buffer), std::vector<argument_type>{}});
							current_context = &contexts.top();
							buffer.clear();
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
							// NOTE: No need to continue since we know that the only way to
							//       this state is from the initial states
							return parse_result{evaluator.evaluate(*current_context), ptr};

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
							contexts.push(parser_context{std::move(buffer), std::vector<argument_type>{}});
							current_context = &contexts.top();
							buffer.clear();
							break;

						case ',':
							current_context->args.push_back(string_converter.convert(std::move(buffer)));
							buffer.clear();
							break;

						case ')':
						{
							if(!buffer.empty())
							{
								current_context->args.push_back(string_converter.convert(std::move(buffer)));
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
							contexts.push(parser_context{std::move(buffer), std::vector<argument_type>{}});
							current_context = &contexts.top();
							buffer.clear();
							current_state = parser_state::read_list_item;
							break;

						case ',':
							current_context->args.push_back(string_converter.convert(std::move(buffer)));
							buffer.clear();
							current_state = parser_state::read_list_item;
							break;

						case ')':
						{
							if(!buffer.empty())
							{
								current_context->args.push_back(string_converter.convert(std::move(buffer)));
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
							contexts.push(parser_context{std::move(buffer), std::vector<argument_type>{}});
							current_context = &contexts.top();
							buffer.clear();
							current_state = parser_state::before_list_item;
							break;

						default:
							if(!is_whitespace(ch_in))
							{ throw input_error{"Unexpected character after command name"}; }
					}
					break;
			}
		}
		return parse_result{string_converter.convert(buffer), ptr};
	}
}

#endif