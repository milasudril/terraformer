#ifndef TERRAFORMER_VALUE_MAPS_ASINH_VALUE_MAP_HPP
#define TERRAFORMER_VALUE_MAPS_ASINH_VALUE_MAP_HPP

#include <cmath>

#include <cstdio>

namespace terraformer::value_maps
{
	class asinh_value_map
	{
	public:
		constexpr explicit asinh_value_map(float scale_factor, float steepness) noexcept:
			m_scale_factor{scale_factor},
			m_steepness{steepness}
		{}

		constexpr float max() const noexcept
		{ return to_value(1.0f); }

		constexpr float min() const noexcept
		{ return -max(); }

		constexpr float scale_factor() const noexcept
		{ return m_scale_factor; }

		constexpr float steepness() const noexcept
		{ return m_steepness; }

		constexpr float from_value(float x) const noexcept
		{
			auto const xi = std::asinh(x/m_scale_factor)/m_steepness;
			return (xi + 1.0f)/2.0f;
		}

		constexpr float to_value(float x) const noexcept
		{
			auto const xi = 2.0f*x - 1.0f;
			return m_scale_factor*std::sinh(m_steepness*xi);
		}

	private:
		float m_scale_factor;
		float m_steepness;
	};
}
#endif