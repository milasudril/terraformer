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

	template<class Func, class Init, class ... Args>
	auto fold_over_line_segments(std::span<location const> locs, Func f, Init&& init, Args... args)
	{
		using ret_type = std::result_of_t<Func(line_segment, Init, Args...)>;
		static_assert(std::is_same_v<ret_type, Init>);

		if(std::size(locs) < 2)
		{ return ret_type{}; }

		auto ret = f(line_segment{locs[0], locs[1]}, std::forward<Init>(init), args...);

		for(size_t k = 2; k != std::size(locs); ++k)
		{ ret = f(line_segment{locs[k - 1], locs[k]}, std::move(ret), args...); }

		return ret;
	}
}

#endif