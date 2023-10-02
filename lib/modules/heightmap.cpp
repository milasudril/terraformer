//@	{"target": {"name":"heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/common/utils.hpp"

#include "lib/pixel_store/image_io.hpp"

void terraformer::generate(heightmap& hm, initial_heightmap_description const& params, random_generator& rng)
{
	auto& pixels = hm.pixel_storage;
	auto const h = pixels.height();
	auto const w = pixels.width();

	if(h < 2 || w < 2)
	{ throw std::runtime_error{"Output resolution is too small"}; }

	auto const ay = params.main_ridge.ridge_curve_xy.amplitude*static_cast<float>(h - 1)*hm.pixel_size;
	auto const az = params.main_ridge.ridge_curve_xz.amplitude;
	auto const ridge_curve = generate(rng,
		params.main_ridge.ridge_curve_xy.wave,
		output_range{-ay, ay},
		params.main_ridge.ridge_curve_xz.wave,
		output_range{-az, az},
		polyline_location_params{
			.point_count = w,
 			.dx = hm.pixel_size,
			.start_location = terraformer::location{
				0.0f,
				params.main_ridge.ridge_curve_xy.initial_value*static_cast<float>(h - 1)*hm.pixel_size,
				params.main_ridge.ridge_curve_xz.initial_value
			}
		}
	);

	auto const y_south =static_cast<float>(h - 1)*hm.pixel_size;
	auto const ridge_loc = params.main_ridge.ridge_curve_xy.initial_value*y_south;

	grayscale_image u{w, h};
	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			location const loc{
				static_cast<float>(x)*hm.pixel_size,
				static_cast<float>(y)*hm.pixel_size,
				0.0f
			};

			auto const i = std::ranges::min_element(ridge_curve,
				[loc](auto a, auto b){
				return distance_xy(a, loc) < distance_xy(b, loc);
			});

			auto const d_curve = distance_xy(*i, loc);
			auto const y_curve = ridge_curve[x][1];
			auto const side = loc[1] - y_curve;
			auto const d_curve_boundary = side < 0.0f?
				y_curve:
				y_south - y_curve;
			auto const distance = d_curve/d_curve_boundary;
			auto const t = side < 0.0f? loc[1]/y_curve : (loc[1] - y_south)/(y_curve - y_south);
			auto const val = std::sqrt(1.0f - std::lerp(1.0f, distance, t));

			u(x, y) = side < 0.0f? ridge_loc*val : val*ridge_loc + y_south*(1.0f - val);
		}
	}

	grayscale_image v{w, h};
	{
		for(uint32_t y = 0; y != h; ++y)
		{
			auto v_sum = 0.0f;
			for(uint32_t x = 0; x != w; ++x)
			{
				auto const gradvec = grad(std::as_const(u).pixels(), x, y, 1.0f, clamp_at_boundary{});
				auto const v_val = std::clamp(gradvec[1], -hm.pixel_size, hm.pixel_size);
				v(x, y) = v_sum;
				v_sum += v_val;
			}
		}

		auto const range = std::ranges::minmax_element(v.pixels());
		std::ranges::transform(v.pixels(),
			v.pixels().begin(), [
				range = std::ranges::minmax_result{*range.min, *range.max},
				maxval = static_cast<float>(w)*hm.pixel_size
			](auto const val) {
				return maxval*(val - range.min)/(range.max - range.min);
		});
	}

	grayscale_image ns_wave_output{w, h};
	{
		fractal_wave const ns_wave{rng, params.ns_wave.wave.shape};
		auto const wavelength = params.ns_wave.wave.wave_properties.wavelength;
		auto const phase = params.ns_wave.wave.wave_properties.phase;
		auto amplitude = 0.0f;
		for(uint32_t y = 0; y != h; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{
				auto const val = ns_wave(u(x, y)/wavelength + phase);
				amplitude = std::max(std::abs(val), amplitude);
				ns_wave_output(x, y) = val;
			}
		}
		std::ranges::transform(ns_wave_output.pixels(),
			ns_wave_output.pixels().begin(), [
				gain = params.ns_wave.amplitude/amplitude
			](auto const val) {
			return val*gain;
		});
	}

	auto const& corners = params.corners;
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
			auto const south = std::lerp(sw_elev, se_elev, xi);

			auto const ridge_loc_z = ridge_curve[x][2];

			auto const yf = static_cast<float>(y)*hm.pixel_size;
			auto const y_curve = ridge_curve[x][1];
			auto const side = yf - y_curve;
			auto const bump_param = side < 0.0f? u(x, y)/ridge_loc :
				(u(x, y) - y_south)/(ridge_loc - y_south);

			auto const bump = smoothstep(2.0f*(bump_param - 0.5f));

			auto const base_elevation = std::lerp(north, south, eta) + ns_wave_output(x, y);
			pixels(x, y) = std::lerp(base_elevation, ridge_loc_z, bump);
		}
	}

	{
		auto const range = std::ranges::minmax_element(pixels.pixels());
		std::ranges::transform(pixels.pixels(),
			pixels.pixels().begin(), [
				range = std::ranges::minmax_result{*range.min, *range.max},
				output_range = params.output_range
			](auto const val) {
				return std::lerp(output_range.min, output_range.max, (val - range.min)/(range.max - range.min));
		});
	}
}

