//@	{"target": {"name":"heightmap.o"}}

#include "./heightmap.hpp"

#include "lib/common/utils.hpp"

#include "lib/pixel_store/image_io.hpp"

terraformer::grayscale_image terraformer::generate(uint32_t width,
	uint32_t height,
	float pixel_size,
	std::span<location const> ridge_curve,
	float ridge_loc)
{
	auto const y_south =static_cast<float>(height - 1)*pixel_size;
	grayscale_image u{width, height};
	for(uint32_t y = 0; y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{
			location const loc{
				static_cast<float>(x)*pixel_size,
				static_cast<float>(y)*pixel_size,
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
	return u;
}

terraformer::grayscale_image terraformer::generate(span_2d<float const> u,
	float pixel_size,
	float ridge_loc,
	damped_wave_description const& ns_distortion,
	random_generator& rng)
{
	auto const width = u.width();
	auto const height = u.height();
	grayscale_image v{width, height};
	{
		filtered_noise_generator_1d const wave{rng, height, pixel_size, ns_distortion.wave};
		auto const amplitude = ns_distortion.initial_amplitude;
		auto const half_distance = ns_distortion.half_distance;

		for(uint32_t y = 0; y != height; ++y)
		{
			for(uint32_t x = 0; x != width; ++x)
			{
				auto const x_val = pixel_size*static_cast<float>(x);
				auto const y_val = u(x, y) - ridge_loc;
				v(x, y) = x_val + amplitude*wave(y_val)
					*std::exp2(std::min(std::abs(y_val)/half_distance, std::max(16.0f - std::log2(amplitude), 0.0f)));
			}
		}
	}

	return v;
}

terraformer::grayscale_image terraformer::generate(span_2d<float const> u,
	span_2d<float const> v,
	float pixel_size,
	float ridge_loc,
	modulated_damped_wave_description const& ns_wave_desc,
	random_generator& rng)
{
	auto const w = u.width();
	auto const h = u.height();
	grayscale_image ns_wave_output{w, h};

	filtered_noise_generator_1d const wave{rng, h, pixel_size, ns_wave_desc.nominal_oscillations.wave};
	auto const amplitude = ns_wave_desc.nominal_oscillations.initial_amplitude;
	auto const half_distance = ns_wave_desc.nominal_oscillations.half_distance;

	auto const& amp_mod_desc = ns_wave_desc.amplitude_modulation;
	filtered_noise_generator_1d const amp_mod{rng, w, pixel_size, amp_mod_desc.modulating_wave};
	auto const amp_mod_depth = amp_mod_desc.depth;

	auto const& half_distance_mod_desc = ns_wave_desc.half_distance_modulation;
	filtered_noise_generator_1d const half_distance_mod{rng, w, pixel_size, half_distance_mod_desc.modulating_wave};
	auto const half_distance_mod_depth = half_distance_mod_desc.depth;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const y_val = u(x, y) - ridge_loc;
			auto const x_val = v(x, y);

			auto const amp_mod_value = amp_mod(x_val);
			auto const amp_res = std::exp2(amp_mod_depth*amp_mod_value)*amplitude;

			auto const half_distnace_mod_value = half_distance_mod(x_val);
			auto const half_distance_res = std::exp2(half_distance_mod_depth*half_distnace_mod_value)*half_distance;

			auto const z_val = amp_res*wave(y_val)*std::exp2(-std::abs(y_val)/half_distance_res);

			ns_wave_output(x, y) = z_val;
		}
	}
	return ns_wave_output;
}

terraformer::grayscale_image terraformer::generate(span_2d<float const> u,
	span_2d<float const> v,
	float ridge_loc,
	fractal_wave_description_2d const& bump_field_desc,
	random_generator& rng)
{
	auto const w = u.width();
	auto const h = u.height();
	grayscale_image bump_field{w, h};
	auto const lambda_x = bump_field_desc.x_wave.wave_properties.wavelength;
	auto const phase_x = bump_field_desc.x_wave.wave_properties.phase;
	auto const lambda_y = bump_field_desc.y_wave.wave_properties.wavelength;
	auto const phase_y = bump_field_desc.y_wave.wave_properties.phase;

	bump_field_generator gen{rng,
		bump_field_generator::params{
			.x = bump_field_desc.x_wave.shape,
			.y = bump_field_desc.y_wave.shape,
			.symmetry = bump_field_desc.symmetry
		}
	};
	auto min = 2.0f;
	auto max = -2.0f;

	location origin{
		0.5f*static_cast<float>(w),
		ridge_loc,
		0.0f
	};

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const vec = (location{u(x, y), v(x, y), 0.0f} - origin)
				.apply(rotation{geosimd::rotation_angle{geosimd::turns{0.125f}}, geosimd::dimension_tag<2>{}})
				.apply(
					scaling{
						1.0f/lambda_x,
						1.0f/lambda_y,
						1.0f
					}
				)
				+ displacement{
					static_cast<float>(phase_x),
					static_cast<float>(phase_y),
					0.0f
				};

			auto const z_val = gen(vec);

			min = std::min(min, z_val);
			max = std::max(max, z_val);

			bump_field(x, y) = z_val;
		}
	}

	auto const amplitude = 0.5f*(max - min);
	auto const offset = 0.5f*(max + min);

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{ bump_field(x, y) = (bump_field(x, y) - offset)/amplitude; }
	}

	return bump_field;
}

void terraformer::generate(heightmap& hm, initial_heightmap_description const& params)
{
	auto& pixels = hm.pixel_storage;
	auto const h = pixels.height();
	auto const w = pixels.width();

	if(h < 2 || w < 2)
	{ throw std::runtime_error{"Output resolution is too small"}; }

	{
		auto const u = hm.u.pixels();
		auto const ns_wave_output = hm.ns_wave.pixels();
	//	auto const bump_field_output = hm.bump_field.pixels();
	//	auto const bump_field_amplitude = params.bump_field.amplitude;
		auto const ridge_loc = static_cast<float>(params.main_ridge.ridge_curve_xy.initial_value);
		auto const& corners = params.corners;
		auto const nw_elev = corners.nw.z;
		auto const ne_elev = corners.ne.z;
		auto const sw_elev = corners.sw.z;
		auto const se_elev = corners.se.z;
		auto const ridge_curve = std::span{hm.ridge_curve};
		auto const y_south =static_cast<float>(h - 1)*hm.pixel_size;

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
//					+ bump_field_amplitude*bump_field_output(x, y);
			}
		}
	}

	switch(params.output_range.control_mode)
	{
		case elevation_range_control_mode::use_guides_only:
			break;
		case elevation_range_control_mode::normalize_output:
		{
			auto range = std::ranges::minmax_element(pixels.pixels());
			std::ranges::transform(pixels.pixels(),
				pixels.pixels().begin(), [
					range = std::ranges::minmax_result{*range.min, *range.max},
					output_range = params.output_range
				](auto const val) {
					return std::lerp(output_range.min, output_range.max, (val - range.min)/(range.max - range.min));
				}
			);
			break;
		}
		case elevation_range_control_mode::softclamp_output:
		{
			std::ranges::transform(pixels.pixels(),
				pixels.pixels().begin(), [
					output_range = params.output_range
				](auto const val) {
					auto const t = (val - output_range.min)/(output_range.max - output_range.min);
					auto const y = smoothstep(4.0f*std::lerp(-1.0f, 1.0f, t)/6.0f);
					return std::lerp(static_cast<float>(output_range.min), static_cast<float>(output_range.max), y);
				}
			);
			break;
		}
		case elevation_range_control_mode::clamp_output:
			std::ranges::transform(pixels.pixels(),
				pixels.pixels().begin(), [
					output_range = params.output_range
				](auto const val) {
					return std::clamp(val, static_cast<float>(output_range.min), static_cast<float>(output_range.max));
				}
			);
			break;
	}

	store(hm.pixel_storage, "output.exr");
}

