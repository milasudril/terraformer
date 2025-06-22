#ifndef TERRAFORMER_UI_VALUE_MAPS_ATAN_VALUE_MAP_HPP
#define TERRAFORMER_UI_VALUE_MAPS_ATAN_VALUE_MAP_HPP

#include <cmath>
#include <limits>
#include <cstdio>

namespace terraformer::ui::value_maps
{
	class atan_value_map
	{
	public:
		constexpr explicit atan_value_map(float max, bool reverse = false) noexcept:
			m_theta_max{reverse? -std::atan(max): std::atan(max)}
		{}
		
		constexpr float max() const noexcept
		{ return std::tan(m_theta_max); }

		constexpr float min() const noexcept
		{ return -std::tan(m_theta_max); }

		constexpr float from_value(float x) const noexcept
		{ 
			return (std::atan(x) + m_theta_max)/(2.0f*m_theta_max);
		}

		constexpr float to_value(float x) const noexcept
		{
			auto const theta = m_theta_max*std::lerp(-1.0f, 1.0f, x);
			return std::tan(theta); 
			
		}

	private:
		float m_theta_max;
	};
}
#endif
