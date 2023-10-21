//@{"target":{"name":"calculator.o"}}

#include "./calculator.hpp"

#include <charconv>
#include <cmath>

double terraformer::calculator::make_argument(std::string_view str)
{
	// Try to convert str to a number. If the entire string is parsed it is a number. Otherwise,
	// it is an identifier.
	{
		double val{};
		auto const res = std::from_chars(std::begin(str), std::end(str), val);

		if(res.ptr == std::end(str) && res.ec == std::errc{})
		{ return val; }
	}

	// For now, the golden section should be enough, since all angles are normalized to one turn.
	if(str == "phi")
	{ return std::numbers::phi; }

	throw input_error{std::string{"Unknown constant `"}.append(str).append("`")};
}

namespace terraformer
{
	namespace
	{
		double add(std::span<double const> vals)
		{
			auto sum = 0.0;
			for(size_t k = 0; k != std::size(vals); ++k)
			{ sum += vals[k]; }

			return sum;
		}

		double mul(std::span<double const> vals)
		{
			auto prod = 1.0;
			for(size_t k = 0; k != std::size(vals); ++k)
			{ prod *= vals[k]; }

			return prod;
		}

		double addinv(std::span<double const> vals)
		{
			if(std::size(vals) != 1)
			{ throw input_error{"`addinv` requires exactly one argument"}; }

			return -vals[0];
		}

		double mulinv(std::span<double const> vals)
		{
			if(std::size(vals) != 1)
			{ throw input_error{"`mulinv` requires exactly one argument"}; }

			return 1.0/vals[0];
		}

		double sub(std::span<double const> vals)
		{
			if(std::size(vals) != 2)
			{ throw input_error{"`sub` requires exactly two arguments"}; }

			return vals[0] - vals[1];
		}

		double div(std::span<double const> vals)
		{
			if(std::size(vals) != 2)
			{ throw input_error{"`div` requires exactly two arguments"}; }

			return vals[0]/vals[1];
		}

		double complement(std::span<double const> vals)
		{
			if(std::size(vals) != 1)
			{ throw input_error{"`compl` requires exactly one argument"}; }

			return 1.0 - vals[0];
		}


		double square(std::span<double const> vals)
		{
			if(std::size(vals) != 1)
			{ throw input_error{"`square` requires exactly one argument"}; }

			return vals[0]*vals[0];
		}

		double sroot(std::span<double const> vals)
		{
			if(std::size(vals) != 1)
			{ throw input_error{"`sroot` requires exactly one argument"}; }

			return std::sqrt(vals[0]);
		}

		double qroot(std::span<double const> vals)
		{
			if(std::size(vals) != 1)
			{ throw input_error{"`qroot` requires exactly one argument"}; }

			return std::pow(vals[0], 1.0/3.0);
		}

		double nth_root(std::span<double const> vals)
		{
			if(std::size(vals) != 2)
			{ throw input_error{"`nth_root` requires exactly two arguments"}; }

			return std::pow(vals[1], 1.0/vals[0]);
		}

		double exp(std::span<double const> vals)
		{
			if(std::size(vals) == 1)
			{ return std::exp2(vals[0]); }

			if(std::size(vals) == 2)
			{ return std::pow(vals[0], vals[1]); }

			throw input_error{"`exp` requires one or two arguments"};
		}

		double log(std::span<double const> vals)
		{
			if(std::size(vals) == 1)
			{ return std::log2(vals[0]); }

			if(std::size(vals) == 2)
			{ return std::log2(vals[1])/std::log2(vals[0]); }

			throw input_error{"`log` requires one or two arguments"};
		}

		double count(std::span<double const> vals)
		{ return static_cast<double>(std::size(vals)); }

		double aritmean(std::span<double const> vals)
		{
			if(std::size(vals) == 0)
			{ throw input_error{"`aritmean` requires at least one argument"}; }

			return add(vals)/count(vals);
		}

		double geommean(std::span<double const> vals)
		{
			if(std::size(vals) == 0)
			{ throw input_error{"`geommean` requires at least one argument"}; }

			return std::pow(mul(vals), 1.0/count(vals));
		}

		double median(std::vector<double> vals)
		{
			if(std::size(vals) == 0)
			{ throw input_error{"`median` requires at least one argument"}; }

			auto const size = std::size(vals);
			auto const mid = size/2;
			auto const ptr = std::begin(vals) + mid;
			std::nth_element(std::begin(vals), ptr, std::end(vals));

			if(size % 2 != 0)
			{ return *ptr; }
			else
			{
				auto const v1 = *ptr;
				auto const prev = ptr - 1;
				std::nth_element(std::begin(vals), prev, std::end(vals));
				return (v1 + *prev)/2.0;
			}
		}

		double norm(std::span<double const> vals)
		{
			auto sum = 0.0;
			for(size_t k = 0; k != std::size(vals); ++k)
			{ sum += vals[k]*vals[k]; }

			return std::sqrt(sum);
		}

		double rms(std::span<double const> vals)
		{
			if(std::size(vals) == 0)
			{ throw input_error{"`rms` requires at least one argument"}; }

			return norm(vals)/std::sqrt(count(vals));
		}

		double fibseq(std::span<double const> vals)
		{
			if(std::size(vals) != 1)
			{ throw input_error{"`fibseq` requires exactly one argument"}; }

			auto const n = vals[0];

			return (std::pow(std::numbers::phi, n) - std::pow(1.0 - std::numbers::phi, n))/std::sqrt(5.0);
		}

		double mersenneseq(std::span<double const> vals)
		{
			if(std::size(vals) != 1)
			{ throw input_error{"`mersenneseq` requires exactly one argument"}; }

			return std::exp2(vals[0]) - 1.0;
		}
	}
}

double terraformer::calculator::evaluate(calculator::expression_context const& context)
{
	// Arithmetic ops
	if(context.command_name == "add")
	{ return add(context.args); }

	if(context.command_name == "mul")
	{ return mul(context.args); }

	if(context.command_name == "addinv")
	{ return addinv(context.args); }

	if(context.command_name == "mulinv")
	{ return mulinv(context.args); }

	if(context.command_name == "sub")
	{ return sub(context.args); }

	if(context.command_name == "div")
	{ return div(context.args); }

	if(context.command_name == "compl")
	{ return complement(context.args); }

	// Powers and radicals
	if(context.command_name == "square")
	{ return square(context.args); }

	if(context.command_name == "sroot")
	{ return sroot(context.args); }

	if(context.command_name == "qroot")
	{ return qroot(context.args); }

	if(context.command_name == "nth_root")
	{ return nth_root(context.args); }

	if(context.command_name == "exp")
	{ return exp(context.args); }

	if(context.command_name == "log")
	{ return log(context.args); }

	// Statistical functions
	if(context.command_name == "count")
	{ return count(context.args); }

	if(context.command_name == "aritmean")
	{ return aritmean(context.args); }

	if(context.command_name == "geommean")
	{ return geommean(context.args); }

	if(context.command_name == "median")
	{ return median(context.args); }

	if(context.command_name == "norm")
	{ return norm(context.args); }

	if(context.command_name == "rms")
	{ return rms(context.args); }

	// Sequences
	if(context.command_name == "fibseq")
	{ return fibseq(context.args); }

	if(context.command_name == "mersenneseq")
	{ return mersenneseq(context.args); }


	throw input_error{std::string{"Unknown function `"}.append(context.command_name).append("`")};
}