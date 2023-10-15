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

	auto const ay = params.main_ridge.ridge_curve_xy.amplitude;
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
				static_cast<float>(params.main_ridge.ridge_curve_xy.initial_value),
				static_cast<float>(params.main_ridge.ridge_curve_xz.initial_value)
			}
		}
	);

	auto const y_south =static_cast<float>(h - 1)*hm.pixel_size;
	auto const ridge_loc = static_cast<float>(params.main_ridge.ridge_curve_xy.initial_value);

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
		auto const& ns_distortion = params.ns_distortion;
		fractal_wave const wave{rng, ns_distortion.wave.shape};
		auto const wavelength = ns_distortion.wave.wave_properties.wavelength;
		auto const phase = ns_distortion.wave.wave_properties.phase;
		auto const amplitude = ns_distortion.initial_amplitude;
		auto const half_distance = ns_distortion.half_distance;

		for(uint32_t y = 0; y != h; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{
				auto const x_val = hm.pixel_size*static_cast<float>(x);
				auto const y_val = u(x, y) - ridge_loc;
				v(x, y) = x_val + amplitude*wave(y_val/wavelength + phase)
					*std::exp2(std::min(std::abs(y_val)/half_distance, std::max(16.0f - std::log2(amplitude), 0.0f)));
			}
		}
	}

	store(u, "distance_field_u.exr");
	store(v, "distance_field_v.exr");

	grayscale_image ns_wave_output{w, h};
	{
		auto const& ns_wave_desc = params.ns_wave;

		fractal_wave const wave{rng, ns_wave_desc.nominal_oscillations.wave.shape};
		auto const wavelength = ns_wave_desc.nominal_oscillations.wave.wave_properties.wavelength;
		auto const phase = ns_wave_desc.nominal_oscillations.wave.wave_properties.phase;
		auto const amplitude = ns_wave_desc.nominal_oscillations.initial_amplitude;
		auto const half_distance = ns_wave_desc.nominal_oscillations.half_distance;

		auto const& amp_mod_desc = ns_wave_desc.amplitude_modulation;
		fractal_wave const amp_mod{rng, amp_mod_desc.modulating_wave.shape};
		auto const amp_mod_wavelength = amp_mod_desc.modulating_wave.wave_properties.wavelength;
		auto const amp_mod_phase = amp_mod_desc.modulating_wave.wave_properties.phase;
		auto const amp_mod_depth = amp_mod_desc.depth;

		auto const& wavelength_mod_desc = ns_wave_desc.wavelength_modulation;
		fractal_wave const wavelength_mod{rng, wavelength_mod_desc.modulating_wave.shape};
		auto const wavelength_mod_wavelength = wavelength_mod_desc.modulating_wave.wave_properties.wavelength;
		auto const wavelength_mod_phase = wavelength_mod_desc.modulating_wave.wave_properties.phase;
		auto const wavelength_mod_depth = wavelength_mod_desc.depth;

		auto const& half_distance_mod_desc = ns_wave_desc.half_distance_modulation;
		fractal_wave const half_distance_mod{rng, half_distance_mod_desc.modulating_wave.shape};
		auto const half_distance_mod_wavelength = half_distance_mod_desc.modulating_wave.wave_properties.wavelength;
		auto const half_distance_mod_phase = half_distance_mod_desc.modulating_wave.wave_properties.phase;
		auto const half_distance_mod_depth = half_distance_mod_desc.depth;

		for(uint32_t y = 0; y != h; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{
				auto const y_val = u(x, y) - ridge_loc;
				auto const x_val = v(x, y);

				auto const amp_mod_value = amp_mod(x_val/amp_mod_wavelength + amp_mod_phase);
				auto const amp_res = std::exp2(amp_mod_depth*amp_mod_value)*amplitude;

				auto const wavelength_mod_value = wavelength_mod(x_val/wavelength_mod_wavelength
					+ wavelength_mod_phase);
				auto const wavelength_res = std::exp2(wavelength_mod_depth*wavelength_mod_value)*wavelength;

				auto const half_distnace_mod_value = half_distance_mod(x_val/half_distance_mod_wavelength
					+ half_distance_mod_phase);
				auto const half_distance_res = std::exp2(half_distance_mod_depth*half_distnace_mod_value)*half_distance;

				auto const z_val = amp_res*wave(y_val/wavelength_res + phase)*std::exp2(-std::abs(y_val)/half_distance_res);

				ns_wave_output(x, y) = z_val;
			}
		}
	}

	auto const& corners = params.corners;
	auto const nw_elev = corners.nw.z;
	auto const ne_elev = corners.ne.z;
	auto const sw_elev = corners.sw.z;
	auto const se_elev = corners.se.z;

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

			auto const base_elevation = std::lerp(north, south, eta);
			pixels(x, y) = std::lerp(base_elevation, ridge_loc_z, bump)
				+ ns_wave_output(x, y);
		}
	}

#if 0
	{
		auto const range = std::ranges::minmax_element(pixels.pixels());
		std::ranges::transform(pixels.pixels(),
			pixels.pixels().begin(), [
				range = std::ranges::minmax_result{*range.min, *range.max},
				output_range = params.output_range
			](auto const val) {
				return std::lerp(output_range.min, output_range.max, (val - range.min)/(range.max - range.min));
			}
		);
	}
#endif
	store(pixels, "output.exr");
}

