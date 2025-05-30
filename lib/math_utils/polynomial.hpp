#ifndef TERRAFORMER_POLYNOMIAL_HPP
#define TERRAFORMER_POLYNOMIAL_HPP

#include <array>
#include <cstddef>

namespace terraformer
{
	template<size_t Degree>
	struct polynomial
	{
		constexpr auto operator()(float x) const
		{
			auto ret = 0.0f;
			auto val = 1.0f;
			for(size_t k = 0; k != std::size(coefficients); ++k)
			{
				ret += coefficients[k]*val;
				val *= x;
			}
			return ret;
		}

		static constexpr auto degree()
		{ return Degree; }

		constexpr auto derivative() const
		{
			if constexpr(Degree != 0)
			{
				polynomial<Degree - 1> ret;
				for(size_t k = 0; k != std::size(ret.coefficients); ++k)
				{ ret.coefficients[k] = (static_cast<float>(k) + 1.0f)*coefficients[k + 1]; }

				return ret;
			}
			else
			{ return polynomial<0>{}; }
		}

		constexpr auto derivative(float x) const
		{ return derivative()(x); }

		constexpr bool operator==(polynomial const& other) const = default;

		constexpr bool operator!=(polynomial const& other) const = default;

		std::array<float, Degree + 1> coefficients;
	};

	template<class ... T>
	requires (sizeof...(T) > 0)
	polynomial(T...) -> polynomial<sizeof...(T) - 1>;

	template<size_t Degree1, size_t Degree2>
	constexpr auto operator*(polynomial<Degree1> const& a, polynomial<Degree2> const& b)
	{
		polynomial<Degree1 + Degree2> ret{};
		for(size_t k = 0; k != std::size(a.coefficients); ++k)
		{
			for(size_t l = 0; l != std::size(b.coefficients); ++l)
			{ ret.coefficients[k + l] += a.coefficients[k]*b.coefficients[l]; }
		}
		return ret;
	}

	template<size_t Degree1, size_t Degree2>
	constexpr auto operator+(polynomial<Degree1> const& a, polynomial<Degree2> const& b)
	{
		polynomial<std::max(Degree1, Degree2)> ret{};
		for(size_t k = 0; k != std::size(ret.coefficients); ++k)
		{
			ret.coefficients[k] = (k < std::size(a.coefficients))? a.coefficients[k] : 0.0f;
			ret.coefficients[k] += (k < std::size(b.coefficients))? b.coefficients[k] : 0.0f;
 		}

		return ret;
	}
}
#endif