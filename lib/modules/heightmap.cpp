//@	{"target": {"name":"heightmap.o"}}

#include "./heightmap.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/smoothstep.hpp"
#include "lib/math_utils/cubic_spline.hpp"

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
	filtered_noise_1d_generator const& wave)
{
	auto const width = u.width();
	auto const height = u.height();
	grayscale_image v{width, height};
	{
		auto const amplitude = ns_distortion.initial_amplitude;
		auto const peak_location = ns_distortion.peak_location;
		auto const half_distance = ns_distortion.half_distance;
		auto const y_dir = ns_distortion.flip_direction? -1.0f : 1.0f;
		auto const x_dir = ns_distortion.invert_displacement? -1.0f : 1.0f;

		for(uint32_t y = 0; y != height; ++y)
		{
			for(uint32_t x = 0; x != width; ++x)
			{
				auto const x_val = pixel_size*static_cast<float>(x);
				auto const y_val = u(x, y) - ridge_loc;
				auto const scale = std::exp2(std::min(std::abs(y_val)/half_distance, 16.0f));
				v(x, y) = x_val + x_dir*scale*amplitude*wave(y_dir*(y_val - peak_location)/scale);
			}
		}
	}

	return v;
}

terraformer::grayscale_image terraformer::generate(
	filtered_noise_2d_generator const& wave,
	span_2d<float const> u,
	span_2d<float const> v,
	float ridge_loc,
	bump_field_description const& bump_field_desc)
{
	auto const w = u.width();
	auto const h = u.height();
	grayscale_image bump_field{w, h};

	displacement const peak_loc{
		static_cast<float>(bump_field_desc.peak_loc_x),
		ridge_loc + static_cast<float>(bump_field_desc.peak_loc_y),
		0.0f
	};

	auto const rot_x = 2.0f*std::numbers::pi_v<float>*bump_field_desc.rotation;
	auto const rot_y = 2.0f*std::numbers::pi_v<float>*
		(bump_field_desc.rotation - 0.25f + bump_field_desc.axis_angle);

	displacement const x_hat{
		std::cos(rot_x),
		std::sin(rot_x),
		0.0f
	};

	displacement const y_hat{
		-std::sin(rot_y),
		std::cos(rot_y),
		0.0f
	};

	auto const det_mat = x_hat[0]*y_hat[1] - x_hat[1]*y_hat[0];
	cubic_spline_control_point const min_param{
		.y = -1.0f,
		.ddx = bump_field_desc.valley_gradient
	};

	cubic_spline_control_point const max_param{
		.y = 1.0f,
		.ddx = bump_field_desc.peak_gradient
	};

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			displacement const r{
				v(x, y),
				u(x, y),
				0.0f
			};
			auto const sample_at = r - peak_loc;
			auto const sample_at_x = inner_product(x_hat, sample_at)/det_mat;
			auto const sample_at_y = inner_product(y_hat, sample_at)/det_mat;

			auto const val = wave(sample_at_x, sample_at_y);
			auto const bias = 1.0f;
			auto const val_with_bias = val + bias;

			bump_field(x, y) = interp(min_param, max_param, 0.5f*val_with_bias);
		}
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

	auto const u = hm.u.pixels();
	auto const v = hm.v.pixels();
	auto const pixel_size = hm.pixel_size;

	auto const bump_field_output = hm.bump_field.pixels();
	auto const bump_field_amplitude = params.bump_field.amplitude;
	auto const ridge_loc = static_cast<float>(params.main_ridge.ridge_curve_xy.initial_value);

	auto const& corners = params.corners;
	cubic_spline_control_point const nw_we{
		.y = corners.nw.z,
		.ddx = std::atan(2.0f*std::numbers::pi_v<float>*corners.nw.slope_x)*static_cast<float>(w)*hm.pixel_size
	};

	cubic_spline_control_point const ne_we{
		.y = corners.ne.z,
		.ddx = std::atan(2.0f*std::numbers::pi_v<float>*corners.ne.slope_x)*static_cast<float>(w)*hm.pixel_size
	};

	cubic_spline_control_point const sw_we{
		.y = corners.sw.z,
		.ddx = std::atan(2.0f*std::numbers::pi_v<float>*corners.sw.slope_x)*static_cast<float>(w)*hm.pixel_size
	};

	cubic_spline_control_point const se_we{
		.y = corners.se.z,
		.ddx = std::atan(2.0f*std::numbers::pi_v<float>*corners.se.slope_x)*static_cast<float>(w)*hm.pixel_size
	};

	auto const nw_ddy = std::atan(2.0f*std::numbers::pi_v<float>*corners.nw.slope_y)*static_cast<float>(h)*hm.pixel_size;

	auto const ne_ddy = std::atan(2.0f*std::numbers::pi_v<float>*corners.ne.slope_y)*static_cast<float>(h)*hm.pixel_size;

	auto const sw_ddy = std::atan(2.0f*std::numbers::pi_v<float>*corners.sw.slope_y)*static_cast<float>(h)*hm.pixel_size;

	auto const se_ddy = std::atan(2.0f*std::numbers::pi_v<float>*corners.se.slope_y)*static_cast<float>(h)*hm.pixel_size;

	auto const ridge_curve = std::span{hm.ridge_curve};
	auto const ridge_slope = std::atan(2.0f*params.main_ridge.slope_y*std::numbers::pi_v<float>);
	auto const y_south =static_cast<float>(h - 1)*hm.pixel_size;

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const xi = static_cast<float>(v(x, y))/(pixel_size*static_cast<float>(w - 1));
			auto const eta = static_cast<float>(u(x, y))/(pixel_size*static_cast<float>(h - 1));

			cubic_spline_control_point const north{
				.y = interp(nw_we, ne_we, xi),
				.ddx = std::lerp(nw_ddy, ne_ddy, xi)
			};

			cubic_spline_control_point const south{
				.y = interp(sw_we, se_we, xi),
				.ddx = std::lerp(sw_ddy, se_ddy, xi)
			};

			auto const ridge_loc_z = ridge_curve[x][2];
			auto const yf = static_cast<float>(y)*hm.pixel_size;
			auto const y_curve = ridge_curve[x][1];
			auto const side = yf - y_curve;
			// TODO: Fix divide-by-zero issue
			auto const bump_param = side < 0.0f? u(x, y)/ridge_loc :
				(u(x, y) - y_south)/(ridge_loc - y_south);

			cubic_spline_control_point const ridge{
				.y = ridge_loc_z,
				.ddx = ridge_slope*(side < 0.0f? ridge_loc: y_south - ridge_loc)
			};

			cubic_spline_control_point const valley{
				.y = 0.0f,
				.ddx = 0.0f
			};

			auto const bump = interp(valley, ridge, bump_param);

			auto const base_elevation = interp(north, south, eta);
			pixels(x, y) = base_elevation
				+ bump
				+ bump_field_amplitude*bump_field_output(x, y);
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

