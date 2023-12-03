//@	{"target":{"name":"curve_displace.o"}}

#include "./curve_displace.hpp"

#include <cassert>

void terraformer::displace(std::span<location> c, std::span<displacement const> dx)
{
	assert(std::size(c) == std::size(dx));
	assert(std::size(c) >= 3);
	auto c_distance = 0.0;
	auto dx_distance = 0.0;
	for(size_t k = 1; k != std::size(c) - 1; ++k)
	{
		c_distance += distance(c[k], c[k - 1]);
		dx_distance += dx[k][0] - dx[k - 1][0];
#if 0
		auto const n = curve_vertex_normal(c[k - 1], c[k], c[k + 1]);
#endif
	}
}
