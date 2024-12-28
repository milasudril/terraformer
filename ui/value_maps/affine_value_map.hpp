#include <cmath>

#ifndef TERRAFORMER_UI_VALUE_MAPS_AFFINE_VALUE_MAP_HPP
#define TERRAFORMER_UI_VALUE_MAPS_AFFINE_VALUE_MAP_HPP

namespace terraformer::ui::value_maps
{
	class affine_value_map
	{
	public:
		constexpr explicit affine_value_map(float min, float max) noexcept:
			m_min{min},
			m_max{max}
		{}

		constexpr float min() const noexcept
		{ return m_min; }

		constexpr float max() const noexcept
		{ return m_max; }

		constexpr float from_value(float x) const noexcept
		{ return std::lerp(0.0f, 1.0f, (x - m_min)/(m_max - m_min)); }

		constexpr float to_value(float x) const noexcept
		{ return std::lerp(m_min, m_max, x); }

	private:
		float m_min;
		float m_max;
	};
}
#endif