#ifndef TERRAFORMER_EXPRESSION_EVALUATOR_HPP
#define TERRAFORMER_EXPRESSION_EVALUATOR_HPP

#include <string_view>
#include <stack>
#include <string>
#include <vector>
#include <span>
#include <stdexcept>

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

	struct parser_context
	{
		std::string command_name;
		std::vector<double> args;
	};

//"  foo ( 1,  3, bar( kaka(5)) , 7)"
	double evaluate(std::string_view expression)
	{
		auto ptr = std::begin(expression);
		auto current_state = parser_state::init;

		std::string buffer;
		std::stack<parser_context> contexts;
		parser_context* current_context = nullptr;
		while(ptr != std::end(expression))
		{
			auto ch_in = *ptr;
			++ptr;
			switch(current_state)
			{
				case parser_state::init:
					if(is_delimiter(ch_in))
					{ throw std::runtime_error{"Unexpected delimiter 1"}; }

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
							contexts.push(parser_context{std::move(buffer), std::vector<double>{}});
							current_context = &contexts.top();
							printf("<%s>\n", current_context->command_name.c_str());
							buffer.clear();
							current_state = parser_state::before_list_item;
							break;

						case ',':
							throw std::runtime_error{"Unexpected delimiter"};

						case ')':
							throw std::runtime_error{"Unexpected delimiter"};

						default:
							if(is_whitespace(ch_in))
							{ current_state = parser_state::after_command_name; }
							else
							{ buffer += ch_in; }
					}
					break;

				case parser_state::before_list_item:
					if(is_delimiter(ch_in))
					{ throw std::runtime_error{"Unexpected delimiter"}; }

					if(!is_whitespace(ch_in))
					{
						buffer += ch_in;
						current_state = parser_state::read_list_item;
					}
					break;

				case parser_state::read_list_item:
					switch(ch_in)
					{
						case '(':
							contexts.push(parser_context{std::move(buffer), std::vector<double>{}});
							current_context = &contexts.top();
							printf("<%s>\n", current_context->command_name.c_str());
							buffer.clear();
							break;

						case ',':
							printf(", [%s]\n", buffer.c_str());
							buffer.clear();
							break;

						case ')':
							printf(") [%s]\n", buffer.c_str());
							buffer.clear();
							current_state = parser_state::after_command;
							printf("</%s>\n", current_context->command_name.c_str());
							contexts.pop();
							current_context = &contexts.top();
							break;

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
							contexts.push(parser_context{std::move(buffer), std::vector<double>{}});
							current_context = &contexts.top();
							printf("<%s>\n", current_context->command_name.c_str());
							buffer.clear();
							current_state = parser_state::read_list_item;
							break;

						case ',':
							printf(", [%s]\n", buffer.c_str());
							buffer.clear();
							current_state = parser_state::read_list_item;
							break;

						default:
							throw std::runtime_error{"Unexpected character"};
					}
					break;

				case parser_state::after_command:
					switch(ch_in)
					{
						case ',':
							current_state = parser_state::read_list_item;
							break;

						case ')':
							printf("</%s>\n", current_context->command_name.c_str());
							contexts.pop();
							current_context = &contexts.top();
							break;

						default:
							if(!is_whitespace(ch_in))
							{ throw std::runtime_error{"Unexpected character"}; }
					}
					break;

				case parser_state::after_command_name:
					switch(ch_in)
					{
						case '(':
							contexts.push(parser_context{std::move(buffer), std::vector<double>{}});
							current_context = &contexts.top();
							printf("<%s>\n", current_context->command_name.c_str());
							buffer.clear();
							current_state = parser_state::before_list_item;
							break;

						default:
							if(!is_whitespace(ch_in))
							{ throw std::runtime_error{"Unexpected character"}; }
					}
					break;
			}
		}
		putchar('\n');
		return 0.0;
	}
}

#endif