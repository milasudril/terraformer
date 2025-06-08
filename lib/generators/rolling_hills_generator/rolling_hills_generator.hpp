//@	{"dependencies_extra": [{"ref": "./rolling_hills_generator.o", "rel": "implementation"}]}"}}

#ifndef TERRAFORMER_ROLLING_HILLS_GENERATOR_HPP
#define TERRAFORMER_ROLLING_HILLS_GENERATOR_HPP

#include "lib/generators/domain/domain_size.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/common/interval.hpp"
#include "lib/common/bounded_value.hpp"

namespace terraformer
{
	struct rolling_hills_filter_descriptor
	{
		float wavelength_x = 4096.0f;
		float wavelength_y = 4096.0f;
		float lf_rolloff = 2.0f;
		float hf_rolloff = 2.0f;
		float y_direction = 0.0f;
	};

	struct rolling_hills_normalized_filter_descriptor
	{
		uint32_t width;
		uint32_t height;
		float f_x;
		float f_y;
		float lf_rolloff;
		float hf_rolloff;
		float y_direction;
	};

	rolling_hills_normalized_filter_descriptor make_rolling_hills_normalized_filter_descriptor(
		domain_size_descriptor const& size,
		rolling_hills_filter_descriptor const& params
	);

	struct rolling_hills_smooth_clamp_descriptor
	{
		float scale;
		float offset;
		float k;

		float min() const
		{ return offset - scale; }
	};

	float clamp(float value, rolling_hills_smooth_clamp_descriptor const& params);

	struct rolling_hills_clamp_to_descriptor
	{
		closed_closed_interval<float> input_range;
		bounded_value<open_open_interval{0.0f, 1.0f}, 1.0f - 1.0f/128.0f> hardness;
	};

	rolling_hills_smooth_clamp_descriptor make_rolling_hills_smooth_clamp_descriptor(
		rolling_hills_clamp_to_descriptor const& params
	);

	struct rolling_hills_shape_descriptor
	{
		closed_closed_interval<float> clamp_to{-1.0f, 1.0f};
		bounded_value<open_open_interval{0.0f, 1.0f}, 1.0f - 1.0f/128.0f> clamp_hardness;
		closed_closed_interval<float> input_mapping{0.0f, 1.0f};
		float exponent = 2.0f;
	};

	struct rolling_hills_descriptor
	{
		std::array<std::byte, 16> rng_seed{};
		rolling_hills_filter_descriptor filter;
		rolling_hills_shape_descriptor shape;
		float amplitude = 4096.0f/(4.0f*std::numbers::pi_v<float>);
		float relative_z_offset = 0.0f;
	};

	grayscale_image generate(domain_size_descriptor const& dom_size, rolling_hills_descriptor const& params);
}

#endif