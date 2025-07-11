#ifndef TERRAFORMER_VALUE_MAPS_SQRT_VALUE_MAP_HPP
#define TERRAFORMER_VALUE_MAPS_SQRT_VALUE_MAP_HPP

#include <cmath>

#include <cstdio>

namespace terraformer::value_maps
{
	class sqrt_value_map
	{
	public:
		constexpr explicit sqrt_value_map(float max_value) noexcept:
			m_max_value{max_value}
		{}

		constexpr float max() const noexcept
		{ return m_max_value; }

		constexpr float min() const noexcept
		{ return -max(); }

		constexpr float from_value(float x) const noexcept
		{
			auto const tmp = x < 0.0f?
				-std::sqrt(-x/m_max_value):
				std::sqrt(x/m_max_value);

			return 0.5f*(tmp + 1.0f);
		}

		constexpr float to_value(float x) const noexcept
		{
			auto const xi = 2.0f*x - 1.0f;
			return xi < 0.0f ? -m_max_value*(xi*xi) : m_max_value*(xi*xi);
		}

	private:
		float m_max_value;
	};
}
#endif