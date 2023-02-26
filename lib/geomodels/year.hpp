#ifndef TERRAFORMER_LIB_YEAR_HPP
#define TERRAFORMER_LIB_YEAR_HPP

#include <compare>

namespace terraformer
{
	class year
	{
	public:
		constexpr explicit year(double value):m_value{value}
		{}

		constexpr double value() const { return m_value; }

		constexpr year& operator*=(double factor)
		{
			m_value *= factor;
			return *this;
		}

		constexpr year& operator/=(double factor)
		{
			m_value /= factor;
			return *this;
		}

		constexpr year& operator+=(year other)
		{
			m_value += other.value();
			return *this;
		}

		constexpr year& operator-=(year other)
		{
			m_value += other.value();
			return *this;
		}

		constexpr auto operator<=>(year const&) const = default;

	private:
		double m_value;
	};


	inline constexpr year operator*(double factor, year a)
	{
		return a*=factor;
	}

	inline constexpr year operator/(year a, double factor)
	{
		return a/factor;
	}

	inline constexpr year operator+(year a, year b)
	{
		return a+=b;
	}

	inline constexpr year operator-(year a, year b)
	{
		return a-=b;
	}
}

#endif