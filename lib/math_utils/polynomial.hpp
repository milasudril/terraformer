#ifndef TERRAFORMER_POLYNOMIAL_HPP
#define TERRAFORMER_POLYNOMIAL_HPP

#include <array>

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

		constexpr auto derivative() const
		{
			polynomial<Degree - 1> ret;
			for(size_t k = 0; k != std::size(ret); ++k)
			{ ret.coefficients[k] = (static_cast<float>(k) + 1.0f)*coefficients[k + 1]; }

			return ret;
		}

		constexpr bool operator==(polynomial const& other) = default;

		constexpr bool operator!=(polynomial const& other) = default;

		std::array<float, Degree + 1> coefficients;
	};
}
#endif