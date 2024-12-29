#ifndef TERRAFORMER_UI_VALUE_MAPS_ASINH_VALUE_MAP_HPP
#define TERRAFORMER_UI_VALUE_MAPS_ASINH_VALUE_MAP_HPP

#include <cmath>

namespace terraformer::ui::value_maps
{
	class asinh_value_map
	{
	public:
		constexpr explicit asinh_value_map(float reference_value, float max) noexcept:
			m_reference_value{reference_value},
			m_max{max}
		{}

		constexpr float max() const noexcept
		{ return m_reference_value*std::sinh(m_max)/std::sinh(1.0f); }

		constexpr float from_value(float x) const noexcept
		{
			auto const xi = std::asinh(std::sinh(1.0f)*x/m_reference_value)/m_max;
			return (xi + 1.0f)/2.0f;
		}

		constexpr float to_value(float x) const noexcept
		{
			auto const xi = 2.0f*x - 1.0f;
			return m_reference_value*std::sinh(m_max*xi)/std::sinh(1.0f);
		}

	private:
		float m_reference_value;
		float m_max;
	};
}
#endif