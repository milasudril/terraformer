#ifndef TERRAFORMER_UI_VALUE_MAPS_ASINH_VALUE_MAP_HPP
#define TERRAFORMER_UI_VALUE_MAPS_ASINH_VALUE_MAP_HPP

#include <cmath>

namespace terraformer::ui::value_maps
{
	class asinh_value_map
	{
	public:
		constexpr explicit asinh_value_map(float max) noexcept:
			m_max{max}
		{}

		constexpr float max() const noexcept
		{ return m_max; }

		constexpr float from_value(float x) const noexcept
		{
			auto const xi = std::asinh(std::lerp(-std::sinh(2.0f), std::sinh(2.0f), 0.5f*(x/m_max + 1.0f)))/2.0f;
			return 0.5f*(xi + 1.0f);
		}

		constexpr float to_value(float x) const noexcept
		{
			auto const xi = 2.0f*x - 1.0f;
			return m_max*std::sinh(2.0f*xi)/std::sinh(2.0f);
		}

	private:
		float m_reference_value;
		float m_max;
	};
}
#endif