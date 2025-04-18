//@	{"target":{"name":"find_zeros.o"}}

#include "./find_zeros.hpp"

#include <algorithm>

terraformer::find_zeros_result terraformer::find_zeros(std::span<float const> data_points, float margin_factor)
{
	if(std::size(data_points) == 0)
	{ return find_zeros_result{std::vector<size_t>{}, 0.0f}; }

	auto const i_peak_valley = std::ranges::minmax_element(data_points);
	auto const amplitude = 0.5f*(*i_peak_valley.max - *i_peak_valley.min);

	auto const threshold = margin_factor*amplitude;

	auto const i_first_nonzero = std::ranges::find_if(data_points, [threshold](auto x) {
		return std::abs(x) > threshold;
	});
	if(i_first_nonzero == std::end(data_points))
	{ return find_zeros_result{std::vector<size_t>{}, 0.0f}; }

	enum class state:int{going_up, going_down, go_below_margin, go_above_margin};
	auto current_state = *i_first_nonzero > 0.0f ? state::going_down : state::going_up;

	find_zeros_result ret{};
	ret.first_value = *i_first_nonzero;

	for(auto i = i_first_nonzero; i != std::end(data_points); ++i)
	{
		switch(current_state)
		{
			case state::going_down:
				if(*i <= 0.0f)
				{
					ret.zeros.push_back(i - std::begin(data_points));
					current_state = state::go_below_margin;
				}
				break;

			case state::going_up:
				if(*i >= 0.0f)
				{
					ret.zeros.push_back(i - std::begin(data_points));
					current_state = state::go_above_margin;
				}
				break;

			case state::go_below_margin:
				if(*i < -threshold)
				{ current_state = state::going_up; }
				break;

			case state::go_above_margin:
				if(*i > threshold)
				{ current_state = state::going_down; }
				break;
			}
	}
	return ret;
}
