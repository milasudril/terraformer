#ifndef TERRAFORMER_UI_VALUE_MAPS_LOG_VALUE_MAP_HPP
#define TERRAFORMER_UI_VALUE_MAPS_LOG_VALUE_MAP_HPP

#include "./affine_value_map.hpp"

#include <cmath>

namespace terraformer::ui::value_maps
{
	class log_value_map
	{
	public:
		constexpr explicit log_value_map(float min, float max, float base) noexcept:
			m_affie{min, max},
			m_base{std::log2(base)}
		{}

		constexpr float min() const noexcept
		{ return std::exp2(m_affie.min()*m_base); }

		constexpr float max() const noexcept
		{ return std::exp2(m_affie.max()*m_base); }

		constexpr float base() const noexcept
		{ return m_base; }

		constexpr float from_value(float x) const noexcept
		{ return m_affie.from_value(std::log2(x)/m_base); }

		constexpr float to_value(float x) const noexcept
		{ return std::exp2(m_base*m_affie.to_value(x)); }

	private:
		affine_value_map m_affie;
		float m_base;
	};
}
#endif