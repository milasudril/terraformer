#ifndef TERRAFORMER_POLYNOMIAL_HPP
#define TERRAFORMER_POLYNOMIAL_HPP

#include <array>
#include <cstddef>
#include <functional>

namespace terraformer
{
	template<class Value, size_t Degree>
	struct polynomial
	{
		template<class Param>
		constexpr auto operator()(Param x) const
		{
			if constexpr(Degree == 4)
			{
				auto const x2 = x*x;
				auto const x3 = x*x2;
				auto const x4 = x2*x2;
				return coefficients[0] + x*coefficients[1] + x2*coefficients[2] + x3*coefficients[3] + x4*coefficients[4];
			}
			else
			if constexpr(Degree == 5)
			{
				auto const x2 = x*x;
				auto const x3 = x*x2;
				auto const x4 = x2*x2;
				auto const x5 = x2*x3;
				return coefficients[0] + x*coefficients[1] + x2*coefficients[2] + x3*coefficients[3] + x4*coefficients[4] + x5*coefficients[5];
			}
			else
			{
				Value ret{};
				Param param{1};
				for(size_t k = 0; k != std::size(coefficients); ++k)
				{
					ret += coefficients[k]*param;
					param *= x;
				}
				return ret;
			}
		}

		static constexpr auto degree()
		{ return Degree; }

		constexpr auto derivative() const
		{
			if constexpr(Degree != 0)
			{
				polynomial<Value, Degree - 1> ret;
				for(size_t k = 0; k != std::size(ret.coefficients); ++k)
				{ ret.coefficients[k] = (static_cast<float>(k) + 1.0f)*coefficients[k + 1]; }

				return ret;
			}
			else
			{ return polynomial<Value, 0>{}; }
		}

		template<class Param>
		constexpr auto derivative(Param x) const
		{ return derivative()(x); }

		constexpr bool operator==(polynomial const& other) const = default;

		constexpr bool operator!=(polynomial const& other) const = default;

		std::array<Value, Degree + 1> coefficients;
	};

	template<class First, class ... T>
	polynomial(First, T...) -> polynomial<First, sizeof...(T)>;

	template<class Value, size_t Degree1, size_t Degree2, class CoeffMultFunction = std::multiplies<>>
	constexpr auto multiply(
		polynomial<Value, Degree1> const& a,
		polynomial<Value, Degree2> const& b,
		CoeffMultFunction&& multfun = CoeffMultFunction{}
	)
	{
		polynomial<std::invoke_result_t<CoeffMultFunction, Value, Value>, Degree1 + Degree2> ret{};
		for(size_t k = 0; k != std::size(a.coefficients); ++k)
		{
			for(size_t l = 0; l != std::size(b.coefficients); ++l)
			{ ret.coefficients[k + l] += multfun(a.coefficients[k], b.coefficients[l]); }
		}
		return ret;
	}

	template<class Value, size_t Degree, class CoeffMultFunction = std::multiplies<>>
	constexpr auto take_square_of(
		polynomial<Value, Degree> const& input,
		CoeffMultFunction&& multfun = CoeffMultFunction{}
	)
	{	return multiply(input, input, std::forward<CoeffMultFunction>(multfun)); }

	template<class Value, size_t Degree1, size_t Degree2>
	requires requires (Value const& x){
		{x*x};
	}
	constexpr auto operator*(polynomial<Value, Degree1> const& a, polynomial<Value, Degree2> const& b)
	{ return multiply(a, b); }

	template<class Value, size_t Degree1, size_t Degree2>
	constexpr auto operator+(polynomial<Value, Degree1> const& a, polynomial<Value, Degree2> const& b)
	{
		polynomial<Value, std::max(Degree1, Degree2)> ret{};
		for(size_t k = 0; k != std::size(ret.coefficients); ++k)
		{
			ret.coefficients[k] = (k < std::size(a.coefficients))? a.coefficients[k] : Value{};
			ret.coefficients[k] += (k < std::size(b.coefficients))? b.coefficients[k] : Value{};
 		}

		return ret;
	}

	template<class Value, size_t Degree1, size_t Degree2>
	constexpr auto operator-(polynomial<Value, Degree1> const& a, polynomial<Value, Degree2> const& b)
	{
		polynomial<Value, std::max(Degree1, Degree2)> ret{};
		for(size_t k = 0; k != std::size(ret.coefficients); ++k)
		{
			ret.coefficients[k] = (k < std::size(a.coefficients))? a.coefficients[k] : Value{};
			ret.coefficients[k] -= (k < std::size(b.coefficients))? b.coefficients[k] : Value{};
 		}

		return ret;
	}
}
#endif