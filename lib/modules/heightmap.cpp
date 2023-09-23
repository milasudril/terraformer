//@	{"target": {"name":"heightmap.o"}}

#include "./heightmap.hpp"

void terraformer::generate(heightmap& hm, initial_heightmap_description const& params)
{
	auto& pixels = hm.pixel_storage;
	auto const h = pixels.height();
	auto const w = pixels.width();

	if(h < 2 || w < 2)
	{ throw std::runtime_error{"Output resolution is too small"}; }

	auto const corners = params.corners;
	auto const nw_elev = corners.nw.elevation;
	auto const ne_elev = corners.ne.elevation;
	auto const sw_elev = corners.sw.elevation;
	auto const se_elev = corners.se.elevation;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const xi = static_cast<float>(x)/static_cast<float>(w - 1);
			auto const eta = static_cast<float>(y)/static_cast<float>(h - 1);

			auto const north = std::lerp(nw_elev, ne_elev, xi);
			auto const south = std::lerp(se_elev, sw_elev, xi);

			pixels(x, y) = std::lerp(north, south, eta);
		}
	}
}

