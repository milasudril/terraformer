//@	{"target":{"name":"curve_displace.o"}}

#include "./curve_displace.hpp"

#include <cassert>

void terraformer::displace(std::span<location> c, std::span<displacement const> dx)
{
	assert(std::size(c) == std::size(dx));
	assert(std::size(c) >= 3);

	for(size_t k = 1; k != std::size(c) - 1; ++k)
	{
#if 0
		auto const n = curve_vertex_normal(c[k - 1], c[k], c[k + 1]);
#endif
	}
}
