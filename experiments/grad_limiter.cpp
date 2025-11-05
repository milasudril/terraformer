//@	{"target":{"name":"grad_limiter.o"}}

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/interp.hpp"
#include <algorithm>
#include <geosimd/basic_vector.hpp>

float run_pass(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	float pixel_size
)
{
	auto max_grad_out = 0.0f;
	for(int32_t y = 0; y != static_cast<int32_t>(output.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(output.width()); ++x)
		{
			using clamp_tag = terraformer::span_2d_extents::clamp_tag;
			auto const dx = 0.5f*(
				  input(x + 1, y, clamp_tag{})
				- input(x - 1, y, clamp_tag{})
			)/pixel_size;
			auto const dy = 0.5f*(
				  input(x, y + 1, clamp_tag{})
				- input(x, y - 1, clamp_tag{})
			)/pixel_size;
			terraformer::displacement const gradvec{dx, dy, 0.0f};
			auto const grad_squared = norm_squared(gradvec);
			max_grad_out = std::max(grad_squared, max_grad_out);

			if(grad_squared > 1.0f)
			{
				auto const sample_from = terraformer::location{
					static_cast<float>(x),
					static_cast<float>(y),
					0.0f
				} + gradvec/std::sqrt(grad_squared);

				auto const uphill_value = interp(input, sample_from[0], sample_from[1], terraformer::clamp_at_boundary{});
				auto const current_value = input(x, y);
				auto const required_value = (uphill_value - 1.0f*pixel_size)*(1.0f + 1.0f/1024.0f);
				auto const error = required_value - current_value;
				auto const output_val = current_value + error/32.0f;
				output(x, y) = output_val;
			}
			else
			{
				auto const output_val = input(x, y);
				output(x, y) = output_val;
			}
		}
	}

	return max_grad_out;
}

int main()
{
	auto buffer_a = load(
		terraformer::empty<terraformer::grayscale_image>{},
		"experiments/rolling_hills.exr"
	);

	auto buffer_b = terraformer::create_with_same_size(buffer_a.pixels());

	auto pixels_a = buffer_a.pixels();
	auto pixels_b = buffer_b.pixels();

	for(size_t k = 0; k != 8192; ++k)
	{
		auto ret = run_pass(pixels_b, pixels_a, 8);
		if(k % 128 == 0)
		{ printf("%.8g\n", std::sqrt(ret)); }
		if(ret <= 1.0f)
		{ break; }

		std::swap(pixels_a, pixels_b);
	}

	store(pixels_b, "/dev/shm/output.exr");
}
