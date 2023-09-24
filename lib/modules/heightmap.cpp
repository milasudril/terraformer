//@	{"target": {"name":"heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/common/utils.hpp"

#include "lib/pixel_store/image_io.hpp"

void terraformer::generate(heightmap& hm, initial_heightmap_description const& params)
{
	auto& pixels = hm.pixel_storage;
	auto const h = pixels.height();
	auto const w = pixels.width();

	if(h < 2 || w < 2)
	{ throw std::runtime_error{"Output resolution is too small"}; }

	auto const ridge_curve = generate(per_thread_rng,
		params.main_ridge.ridge_curve_xy,
		output_range{-4096.0f/hm.pixel_size, 4096.0f/hm.pixel_size},
		params.main_ridge.ridge_curve_xz,
		output_range{-512.0f, 512.0f},
		polyline_location_params{
			.point_count = w,
 			.dx = hm.pixel_size,
			.start_location = terraformer::location{
				0.0f,
				params.main_ridge.y0*static_cast<float>(h - 1),
				params.main_ridge.z0,
			}
		}
	);

	auto const& corners = params.corners;
	auto const nw_elev = corners.nw.elevation;
	auto const ne_elev = corners.ne.elevation;
	auto const sw_elev = corners.sw.elevation;
	auto const se_elev = corners.se.elevation;
//	auto const ridge_loc_y = params.main_ridge.y0;
//	auto const ridge_loc_z = params.main_ridge.z0;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const xi = static_cast<float>(x)/static_cast<float>(w - 1);
			auto const eta = static_cast<float>(y)/static_cast<float>(h - 1);

			auto const north = std::lerp(nw_elev, ne_elev, xi);
			auto const south = std::lerp(sw_elev, se_elev, xi);

			auto const ridge_loc_y = ridge_curve[x][1]/static_cast<float>(h - 1);
			auto const ridge_loc_z = ridge_curve[x][2];

			auto const side = eta - ridge_loc_y;
			auto const t = side < 0.0f?
				1.0f + (eta - ridge_loc_y)/ridge_loc_y:
				1.0f - (eta - ridge_loc_y)/(1.0f - ridge_loc_y);

			auto const bump = smoothstep(2.0f*(t - 0.5f));

			auto const base_elevation = std::lerp(north, south, eta);
			pixels(x, y) = std::lerp(base_elevation, ridge_loc_z, bump);
		}
	}

	store(pixels, "output.exr");
}

