#ifndef TERRAFORMER_LIB_POLYNOMIAL_HPP
#define TERRAFORMER_LIB_POLYNOMIAL_HPP

#include <concepts>
#include <cstddef>
#include <array>
#include <cstdio>

namespace terraformer
{
	template<std::floating_point Precision, size_t Degree>
	class polynomial
	{
	public:
		template<class ... Coeffs>
		requires(sizeof...(Coeffs) == Degree)
		constexpr explicit polynomial(Precision first, Coeffs... vals):m_coeffs{first, vals...}{}

		constexpr Precision operator()(Precision x) const
		{
			auto t = static_cast<Precision>(1.0);
			auto sum = static_cast<Precision>(0.0);
			for(size_t k = 0; k != Degree + 1; ++k)
			{
				sum += m_coeffs[k]*t;
				t *= x;
			}

			return sum;
		}

		constexpr auto& coefficients() const
		{ return m_coeffs; }

		auto& coefficients()
		{ return m_coeffs; }

		static constexpr size_t degree()
		{ return Degree; }

	private:
		std::array<Precision, Degree + 1> m_coeffs;
	};

	template<class First, class ... Coeffs>
	polynomial(First, Coeffs...) -> polynomial<First, sizeof...(Coeffs)>;
}

#endif
