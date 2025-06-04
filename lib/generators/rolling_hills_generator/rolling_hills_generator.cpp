//@	{"target": {"name": "./rolling_hills_generator.o"}}

#include "./rolling_hills_generator.hpp"

#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/dft_engine.hpp"
#include "lib/common/rng.hpp"

#include <cassert>
#include <random>

namespace
{
	terraformer::grayscale_image make_filter(
		uint32_t width,
		uint32_t height,
		float f_x,
		float f_y,
		float theta
	)
	{
		f_x *= 2.0f*std::numbers::pi_v<float>;
		f_y *= 2.0f*std::numbers::pi_v<float>;

		auto const w_float = static_cast<float>(width);
		auto const h_float = static_cast<float>(height);
		auto const x_0 = 0.5f*w_float;
		auto const x_y = 0.5f*h_float;
		terraformer::grayscale_image ret{width, height};
		auto const cos_theta = std::cos(theta);
		auto const sin_theta = std::sin(theta);
		for(uint32_t y = 0; y != height; ++y)
		{
			for(uint32_t x = 0; x != width; ++x)
			{
				auto const xi_in = static_cast<float>(x) - x_0;
				auto const eta_in = static_cast<float>(y) - x_y;

				auto const xi = (xi_in*cos_theta + eta_in*sin_theta)/f_x;
				auto const eta = (-xi_in*sin_theta + eta_in*cos_theta)/f_y;

				auto const r2 = xi*xi + eta*eta;
				auto const r4 = r2*r2;

				ret(x, y) = 2.0f*(1.0f/std::sqrt(1.0f + r4))*(r2/std::sqrt(r4 + 1.0f));

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
	auto const normalized_f_x = size.width/params.wavelength_x;
	auto const normalized_f_y = size.height/params.wavelength_y;
	// Assume a bandwidth of at most 6 octaves = 64 periods
	// Take 4 samples per period
	// This gives a size of 256 pixels, but the size is multiplied by 2 to guarantee an even number.
	// Therefore, use 128 pixels as factor. Notice that if shape is not 1, the required bandwidth
	// is increased. For example, expanding sin^4(x) gives a cos(4x) term
	auto const exponent = params.shape.exponent;
	auto const min_pixel_count = std::exp2(std::abs(std::log2(exponent)))*128.0f*std::max(normalized_f_x, normalized_f_y);

	auto const w_scaled = normalized_f_x > normalized_f_y?
		min_pixel_count: min_pixel_count*size.width/size.height;
	auto const h_scaled = normalized_f_x > normalized_f_y?
		min_pixel_count*size.height/size.width : min_pixel_count;

	auto const w_img = 2u*std::max(static_cast<uint32_t>(w_scaled + 0.5f), 1u);
	auto const h_img = 2u*std::max(static_cast<uint32_t>(h_scaled + 0.5f), 1u);
	auto const wh_ratio = std::max(w_scaled/h_scaled, h_scaled/w_scaled);

	auto const filter = make_filter(
		w_img,
		h_img,
		2.0f*w_scaled*wh_ratio/params.wavelength_x,
		2.0f*h_scaled*wh_ratio/params.wavelength_y,
		2.0f*std::numbers::pi_v<float>*params.filter_orientation
	);

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
	for(uint32_t y = 0; y != h_img; ++y)
	{
		for(uint32_t x = 0; x != w_img; ++x)
		{ max = std::max(max, filtered_output(x, y)); }
	}

	shape_output_range output_range{params.shape};
	auto const amplitude = params.amplitude;
	auto const relative_z_offset = params.relative_z_offset;
	for(uint32_t y = 0; y != h_img; ++y)
	{
			for(uint32_t x = 0; x != w_img; ++x)
			{ filtered_output(x, y) = amplitude*(apply_shape(filtered_output(x, y)/max, params.shape, output_range) + relative_z_offset); }
	}

	return filtered_output;
}
