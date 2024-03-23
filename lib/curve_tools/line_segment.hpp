#ifndef TERRAFORMER_LINE_SEGMENT_HPP
#define TERRAFORMER_LINE_SEGMENT_HPP

#include "lib/common/spaces.hpp"

#include <type_traits>
#include <span>

namespace terraformer
{
	struct line_segment
	{
		location from;
		location to;
	};

	inline auto length(line_segment seg)
	{ return distance(seg.from, seg.to); }

	inline auto length_xy(line_segment seg)
	{ return distance_xy(seg.from, seg.to); }

	inline auto length_squared(line_segment seg)
	{ return distance_squared(seg.from, seg.to); }

	inline auto closest_point(line_segment seg, location loc)
	{
		auto const l2 = length_squared(seg);

		if(l2 == 0.0f) [[unlikely]]
		{ return seg.from; }

		auto const t = std::max(0.0f, std::min(1.0f, inner_product(loc - seg.from, seg.to - seg.from) / l2));
		return seg.from + t*(seg.to - seg.from);
	}

	inline auto distance_squared(line_segment seg, location loc)
	{
		return distance_squared(loc, closest_point(seg, loc));
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

	template<class Val>
	struct compare_over_line_segments_result
	{
		using status_code = Val;
		size_t index;
		status_code value;
	};

	template<class Func, class ... Args>
	auto compare_over_line_segments(std::span<location const> a, std::span<location const> b, Func f, Args... args)
	{
		using ret_type = std::result_of_t<Func(line_segment, line_segment, Args...)>;

		auto const n = std::min(std::size(a), std::size(b));
		if(n < 2)
		{
			return compare_over_line_segments_result{
				n,
				ret_type{}
			};
		}

		for(size_t k = 1; k != n; ++k)
		{
			auto const res = f(line_segment{a[k - 1], a[k]}, line_segment{b[k - 1], b[k]}, args...);
			if(res != ret_type{})
			{
				return compare_over_line_segments_result{
					k,
					res
				};
			}
		}

		return compare_over_line_segments_result{
			n,
			ret_type{}
		};
	}

	inline auto distance_squared(std::span<location const> locs, location loc)
	{
		return fold_over_line_segments(locs, [](auto seg, auto loc, auto ... old_distance) {
			if constexpr(sizeof...(old_distance) == 0)
			{ return distance_squared(seg, loc); }
			else
			{ return std::min(distance_squared(seg, loc), old_distance...); }
		}, loc);
	}
}

#endif