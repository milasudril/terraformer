#ifndef TERRAFORMER_POLYNOMIAL_HPP
#define TERRAFORMER_POLYNOMIAL_HPP

#include <array>
#include <cstddef>

namespace terraformer
{
	template<class Value, size_t Degree>
	struct polynomial
	{
		template<class Param>
		constexpr auto operator()(Param x) const
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

	template<class Value, size_t Degree1, size_t Degree2>
	constexpr auto operator*(polynomial<Value, Degree1> const& a, polynomial<Value, Degree2> const& b)
	{
		polynomial<Value, Degree1 + Degree2> ret{};
		for(size_t k = 0; k != std::size(a.coefficients); ++k)
		{
			for(size_t l = 0; l != std::size(b.coefficients); ++l)
			{ ret.coefficients[k + l] += a.coefficients[k]*b.coefficients[l]; }
		}
		return ret;
	}

	template<class Value, size_t Degree1, size_t Degree2>
	constexpr auto operator+(polynomial<Value, Degree1> const& a, polynomial<Value, Degree2> const& b)
	{
		polynomial<Value, std::max(Degree1, Degree2)> ret{};
		for(size_t k = 0; k != std::size(ret.coefficients); ++k)
		{
			ret.coefficients[k] = (k < std::size(a.coefficients))? a.coefficients[k] : 0.0f;
			ret.coefficients[k] += (k < std::size(b.coefficients))? b.coefficients[k] : 0.0f;
 		}

		return ret;
	}
}
#endif