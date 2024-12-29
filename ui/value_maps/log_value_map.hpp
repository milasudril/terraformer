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
			m_base{std::log2(base)},
			m_affie{std::log2(min)/m_base, std::log2(max)/m_base}
		{}

		constexpr float min() const noexcept
		{ return std::exp2(m_affie.min()*m_base); }

		constexpr float max() const noexcept
		{ return std::exp2(m_affie.max()*m_base); }

		constexpr float base() const noexcept
		{ return m_base; }

		constexpr float from_value(float x) const noexcept
		{
			if( x <= 0.0f ) [[unlikely]]
			{ return from_value(min()); }
			return m_affie.from_value(std::log2(x)/m_base);

		}

		constexpr float to_value(float x) const noexcept
		{ return std::exp2(m_base*m_affie.to_value(x)); }

	private:
		float m_base;
		affine_value_map m_affie;
	};
}
#endif