#ifndef TERRAFORMER_PEAKS_HPP
#define TERRAFORMER_PEAKS_HPP

#include <algorithm>
#include <functional>

namespace terraformer
{
	template<std::ranges::random_access_range R, class Callable, class Compare = std::ranges::less>
	constexpr void for_each_peak(R&& input, Callable&& func, Compare&& cmp = std::ranges::less{})
	{
		if(std::size(input) < 3)  // If there are less than 3 elements, there cannot be any maxima
		{ return; }

		auto const n = std::size(input) - 1;
		for(size_t k = 1; k != n; ++k)
		{
			if(cmp(input[k - 1], input[k]) && cmp(input[k + 1], input[k]))
			{ func(k, input[k]); }
		}
	}
}

#endif