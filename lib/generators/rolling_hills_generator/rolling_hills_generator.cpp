//@	{"target": {"name": "./rolling_hills_generator.o"}}

#include "./rolling_hills_generator.hpp"

#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/dft_engine.hpp"
#include "lib/common/rng.hpp"
#include "lib/math_utils/interp.hpp"

#include <cassert>
#include <random>

namespace
{
	struct normalized_filter_descriptor
	{
		uint32_t width;
		uint32_t height;
		float f_x;
		float f_y;
		float lf_rolloff;
		float hf_rolloff;
		float y_direction;
	};

	normalized_filter_descriptor make_normalized_filter_descriptor(
		terraformer::domain_size_descriptor const& size,
		terraformer::rolling_hills_filter_descriptor const& params
	)
	{
		auto const normalized_f_x = size.width/params.wavelength_x;
		auto const normalized_f_y = size.height/params.wavelength_y;

		// Assume a bandwidth of at most 6 octaves = 64 periods. Take 4 samples per period. With a
		// hf_rolloff of two, this gives a size of 256 pixels, but the size is laterr multiplied by 2 to
		// guarantee an even number. Therefore, use 128 pixels as factor.
		auto const min_pixel_count = 2.0f*128.0f*std::max(normalized_f_x, normalized_f_y)/params.hf_rolloff;

		auto const w_scaled = normalized_f_x > normalized_f_y?
			min_pixel_count: min_pixel_count*size.width/size.height;
		auto const h_scaled = normalized_f_x > normalized_f_y?
			min_pixel_count*size.height/size.width : min_pixel_count;

		auto const w_img = 2u*std::max(static_cast<uint32_t>(w_scaled + 0.5f), 1u);
		auto const h_img = 2u*std::max(static_cast<uint32_t>(h_scaled + 0.5f), 1u);
		auto const wh_ratio = std::max(w_scaled/h_scaled, h_scaled/w_scaled);

		return normalized_filter_descriptor{
			.width = w_img,
			.height = h_img,
			.f_x = 4.0f*w_scaled*wh_ratio*std::numbers::pi_v<float>/params.wavelength_x,
			.f_y = 4.0f*h_scaled*wh_ratio*std::numbers::pi_v<float>/params.wavelength_y,
			.lf_rolloff = params.lf_rolloff,
			.hf_rolloff = params.hf_rolloff,
			.y_direction = 2.0f*std::numbers::pi_v<float>*params.y_direction
		};
	}

	terraformer::grayscale_image make_filter(normalized_filter_descriptor const& params)
	{
		auto const f_x = params.f_x;
		auto const f_y = params.f_y;

		auto const w_float = static_cast<float>(params.width);
		auto const h_float = static_cast<float>(params.height);
		auto const x_0 = 0.5f*w_float;
		auto const x_y = 0.5f*h_float;
		terraformer::grayscale_image ret{params.width, params.height};
		auto const cos_theta = std::cos(params.y_direction);
		auto const sin_theta = std::sin(params.y_direction);
		auto const hf_rolloff = params.hf_rolloff;
		auto const lf_rolloff = params.lf_rolloff;
		for(uint32_t y = 0; y != params.height; ++y)
		{
			for(uint32_t x = 0; x != params.width; ++x)
			{
				auto const xi_in = static_cast<float>(x) - x_0;
				auto const eta_in = static_cast<float>(y) - x_y;

				auto const xi = (xi_in*cos_theta + eta_in*sin_theta)/f_x;
				auto const eta = (-xi_in*sin_theta + eta_in*cos_theta)/f_y;

				auto const r2 = xi*xi + eta*eta;

				auto const lpf = 1.0f/std::sqrt(1.0f + std::pow(r2, hf_rolloff));
				auto const hpf = std::pow(r2, 0.5f*lf_rolloff)/std::sqrt(1.0f + std::pow(r2, lf_rolloff));

				ret(x, y) = 2.0f*lpf*hpf;
			}
		}

		return ret;
	}

	terraformer::basic_image<std::complex<float>>
	make_noise(uint32_t width, uint32_t height, terraformer::rng_seed_type rng_seed)
	{
		terraformer::random_generator rng{rng_seed};
		std::uniform_real_distribution U{0.0f, 1.0f};
		terraformer::basic_image<std::complex<float>> ret{width, height};
		auto sign_y = 1.0f;
		for(uint32_t y = 0; y < height; ++y)
		{
			auto sign_x = 1.0f;
			for(uint32_t x = 0; x < width; ++x)
			{
				ret(x, y) = U(rng) * sign_y * sign_x;
				sign_x *= -1.0f;
			}
			sign_y *= -1.0f;
		}
		return ret;
	}

	float clamp(float val, terraformer::closed_closed_interval<float> range)
	{
		auto const clamped_val = std::clamp(val, range.min(), range.max());
		return std::lerp(-1.0f, 1.0f, (clamped_val - range.min())/(range.max() - range.min()));
	}

	float signed_power(float base, float exponent)
	{
		auto const sign = base >= 0.0f? 1.0f : -1.0f;
		return sign*std::pow(sign*base, exponent);
	}

	struct shape_output_range
	{
		explicit shape_output_range(terraformer::rolling_hills_shape_descriptor const& shape):
			min{signed_power(shape.input_mapping.min(), shape.exponent)},
			max{signed_power(shape.input_mapping.max(), shape.exponent)}
		{}

		float min;
		float max;
	};

	float apply_shape(
		float input_value,
		terraformer::rolling_hills_shape_descriptor const& shape,
		shape_output_range output_range
	)
	{
		auto const x_min = shape.input_mapping.min();
		auto const x_max = shape.input_mapping.max();
		auto const mapped_value = std::lerp(x_min, x_max, 0.5f*(clamp(input_value, shape.clamp_to) + 1.0f));
		auto const shaped_value = signed_power(mapped_value, shape.exponent);
		return std::lerp(-1.0f, 1.0f, (shaped_value - output_range.min)/(output_range.max - output_range.min));
	}
}

terraformer::grayscale_image
terraformer::generate(domain_size_descriptor const& size, rolling_hills_descriptor const& params)
{
	auto const filter = make_filter(make_normalized_filter_descriptor(size, params.filter));

	auto const w_img = filter.width();
	auto const h_img = filter.height();

	auto noise = make_noise(w_img, h_img, std::bit_cast<rng_seed_type>(params.rng_seed));

	dft_execution_plan plan_forward{
		span_2d_extents{
			.width = w_img,
			.height = h_img
		},
		dft_direction::forward
	};

	terraformer::basic_image<std::complex<float>> transformed_input{w_img, h_img};
	plan_forward.execute(std::as_const(noise).pixels().data(), transformed_input.pixels().data());

	for(uint32_t y = 0; y != h_img; ++y)
	{
		for(uint32_t x = 0; x != w_img; ++x)
		{ transformed_input(x, y) *= filter(x, y); }
	}

	dft_execution_plan plan_backward{
		span_2d_extents{
			.width = w_img,
			.height = h_img
		},
		dft_direction::backward
	};

	plan_backward.execute(std::as_const(transformed_input).pixels().data(), noise.pixels().data());

	grayscale_image filtered_output{w_img, h_img};
	auto sign_y = 1.0f;
	for(uint32_t y = 0; y < h_img; ++y)
	{
		auto sign_x = 1.0f;
		for(uint32_t x = 0; x < w_img; ++x)
		{
			filtered_output(x, y) = noise(x, y).real() * sign_x * sign_y;
			sign_x *= -1.0f;
		}
		sign_y *= -1.0f;
	}

	auto max = filtered_output(0, 0);
	auto min = max;
	for(uint32_t y = 0; y != h_img; ++y)
	{
		for(uint32_t x = 0; x != w_img; ++x)
		{
			auto const v = filtered_output(x, y);
			max = std::max(max, v);
			min = std::min(min, v);
		}
	}

	shape_output_range output_range{params.shape};
	auto const shape_scale_factor = std::ceil(std::exp2(std::abs(std::log2(params.shape.exponent))));
	auto const w_out = w_img * static_cast<uint32_t>(shape_scale_factor);
	auto const h_out = h_img * static_cast<uint32_t>(shape_scale_factor);
	grayscale_image ret{w_out, h_out};
	auto const amplitude = params.amplitude;
	auto const relative_z_offset = params.relative_z_offset;
	for(uint32_t y = 0; y != h_out; ++y)
	{
		for(uint32_t x = 0; x != w_out; ++x)
		{
			auto const x_in = static_cast<float>(x)/shape_scale_factor;
			auto const y_in = static_cast<float>(y)/shape_scale_factor;
			auto const input_value = interp(filtered_output, x_in, y_in, wrap_around_at_boundary{});
			auto const normalized_value = 2.0f*(input_value - min)/(max - min) - 1.0f;

			ret(x, y) = amplitude*(apply_shape(normalized_value, params.shape, output_range) + relative_z_offset);
		}
	}

	return ret;
}
