#ifndef TERRAFORMER_UI_MAIN_WIDGET_SIZE_RANGE_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_SIZE_RANGE_HPP

#include <limits>
#include <algorithm>

namespace terraformer::ui::main
{
	class widget_size_range
	{
	public:
		constexpr widget_size_range() = default;

		constexpr explicit widget_size_range(float limit_a, float limit_b):
			m_min{std::min(limit_a, limit_b)},
			m_max{std::max(limit_a, limit_b)}
		{}

		constexpr auto min() const
		{ return m_min; }

		constexpr auto max() const
		{ return m_max; }

	private:
		float m_min = 0.0f;
		float m_max = std::numeric_limits<float>::infinity();
	};

	constexpr widget_size_range merge(widget_size_range a, widget_size_range b)
	{
		auto const min = std::max(a.min(), b.min());
		auto const max_temp = std::min(a.max(), b.max());
		auto const max = std::max(min, max_temp);;
		return widget_size_range{min, max};
	}
}

#endif