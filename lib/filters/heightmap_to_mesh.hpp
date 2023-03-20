#ifndef TERRAFORMER_LIB_HEIGHTMAP_TO_MESH_HPP
#define TERRAFORMER_LIB_HEIGHTMAP_TO_MESH_HPP

#include "lib/mesh_store/mesh.hpp"
#include "lib/common/utils.hpp"

namespace terraformer
{
	template<map_2d<float> Map2d>
	struct heightmap
	{
		Map2d pixels;
		float s_x;
		float s_y;
		float s_z;
	};

	template<map_2d<float> Map>
	mesh create(std::type_identity<mesh>, heightmap<Map> const& heightmap);


}

template<terraformer::map_2d<float> Map>
terraformer::mesh terraformer::create(std::type_identity<mesh>, heightmap<Map> const& heightmap)
{
	mesh ret;

	auto const pixels = heightmap.pixels;
	auto const w = pixels.width();
	auto const h = pixels.height();
	auto const s_x = heightmap.s_x;
	auto const s_y = heightmap.s_y;
	auto const s_z = heightmap.s_z;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const x_loc = s_x*(static_cast<float>(x) - 0.5f*static_cast<float>(w) + 0.5f);
			auto const y_loc = s_y*(0.5f*static_cast<float>(h) - static_cast<float>(y) - 0.5f);
			auto const z_loc = s_z*pixels(x, y);

			auto const dfx = 0.5f*s_z*(pixels((x + 1 + w)%w, y) - pixels((x - 1 + w)%w, y))/s_x;
			auto const dfy = 0.5f*s_z*(pixels(x, (y + 1 + h)%h) - pixels(x, (y - 1 + h)%h))/s_y;
			displacement const normal{-dfx, dfy, 1.0f};

			ret.push_back(vertex{location{x_loc, y_loc, z_loc}, direction{normal}});
		}
	}

	uint32_t y_prev = 0;
	for(uint32_t y = 1; y != h; ++y)
	{
		uint32_t x_prev = 0;
		for(uint32_t x = 1; x != w; ++x)
		{
			face const f1{
				y_prev*w + x,
				y_prev*w + x_prev,
				y*w + x
			};

			face const f2{
				y*w + x_prev,
				y*w + x,
				y_prev*w + x_prev
			};

			ret.insert(f1);
			ret.insert(f2);

			x_prev = x;
		}
		y_prev = y;
	}

	return ret;
}

#endif