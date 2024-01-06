#ifndef TERRAFORMER_LINE_SEGMENT_HPP
#define TERRAFORMER_LINE_SEGMENT_HPP

#include "lib/common/spaces.hpp"

#include <type_traits>
#include <span>

namespace terraformer
{
	struct line_segment
	{
		location a;
		location b;
	};

	inline auto length(line_segment seg)
	{ return distance(seg.a, seg.b); }

	inline auto length_squared(line_segment seg)
	{ return distance_squared(seg.a, seg.b); }

	inline auto distance(line_segment seg, location loc)
	{
		auto const l2 = length_squared(seg);

		if(l2 == 0.0f) [[unlikely]]
		{ return distance(seg.a, loc); }

		auto const t = std::max(0.0f, std::min(1.0f, inner_product(loc - seg.a, seg.b - seg.a) / l2));
		auto const proj = seg.a + t*(seg.b - seg.a);

		return distance(loc, proj);
	}

	template<class Func, class ... Args>
	auto fold_over_line_segments(std::span<location const> locs, Func f, Args... args)
	{
		using ret_type = std::result_of_t<Func(line_segment, Args...)>;

		if(std::size(locs) < 2)
		{ return ret_type{}; }

		auto ret = f(line_segment{locs[0], locs[1]}, args...);

		for(size_t k = 2; k != std::size(locs); ++k)
		{ ret = f(line_segment{locs[k - 1], locs[k]}, args..., std::move(ret)); }

		return ret;
	}

	inline auto distance(std::span<location const> locs, location loc)
	{
		return fold_over_line_segments(locs, [](auto seg, auto loc, auto ... old_distance) {
			if constexpr(sizeof...(old_distance) == 0)
			{ return distance(seg, loc); }
			else
			{ return std::min(distance(seg, loc), old_distance...); }
		}, loc);
	}
}

#endif