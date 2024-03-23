#ifndef TERRAFORMER_STATISTICS_HPP
#define TERRAFORMER_STATISTICS_HPP

#include <ranges>
#include <cassert>
#include <algorithm>

namespace terraformer
{
	template<std::ranges::sized_range R>
	auto arithmetic_mean(R&& r)
	{
		using range_value_type = std::ranges::range_value_t<R>;

		range_value_type ret{};
		auto i = std::begin(r);
		while(i != std::end(r))
		{
			ret += *i;
			++i;
		}

		return ret/static_cast<range_value_type>(static_cast<size_t>(std::size(r)));
	}

	template<std::ranges::random_access_range R>
	requires std::sortable<std::ranges::iterator_t<R>, std::ranges::less, std::identity>
	auto percentile(R&& r, std::ranges::range_value_t<R> factor)
	{
		using range_value_type = std::ranges::range_value_t<R>;
		auto const element =
			static_cast<size_t>(factor*static_cast<range_value_type>(static_cast<size_t>(std::size(r))));

		std::ranges::nth_element(r, std::begin(r) + element);
		return *(std::begin(r) + element);
	}
}

#endif