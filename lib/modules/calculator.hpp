//@	{"dependencies_extra":[{"ref": "./calculator.o","rel":"implementation"}]}

#ifndef TERRAFORMER_CALCULATOR_HPP
#define TERRAFORMER_CALCULATOR_HPP

#include "lib/expression_evaluator/expression_evaluator.hpp"
#include "lib/common/utils.hpp"

namespace terraformer
{
	class calculator
	{
		struct expression_context
		{
			std::string command_name;
			std::vector<double> args;
		};

	public:
		static std::string convert(float value)
		{ return to_string_helper(value); }

		float convert(std::string_view str) const
		{
			auto res = expression_evaluator::parse(str, *this);
			if(res.expression_end != std::end(str))
			{ throw input_error{"Junk after expression"}; }

			return static_cast<float>(res.result);
		}

		static double make_argument(std::string_view str);

		static expression_context make_context(std::string&& command_name)
		{ return expression_context{std::move(command_name), std::vector<double>{}}; }

		static double evaluate(expression_context const&);
	};
}

#endif
