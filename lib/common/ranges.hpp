#ifndef TERRAFORMER_RANGES_HPP
#define TERRAFORMER_RANGES_HPP

#include <ranges>
#include <cstdint>

namespace terraformer
{
	template<class T, class U>
	concept random_access_input_range_2d = requires(T r, uint32_t x, uint32_t y)
	{
		{r.width()} -> std::same_as<uint32_t>;
		{r.height()} -> std::same_as<uint32_t>;
		{r(x, y)} -> std::convertible_to<U const&>;
	};
}

#endif