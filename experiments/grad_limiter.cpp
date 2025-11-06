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

			std::array const neighbours{
				input(x + 1, y, clamp_tag{}),
				input(x, y + 1, clamp_tag{}),
				input(x - 1, y, clamp_tag{}),
				input(x, y - 1, clamp_tag{})
			};

			auto const heighest_neighbour = *std::ranges::max_element(neighbours);
			auto const current_value = input(x, y);
			auto const dz_max = heighest_neighbour - current_value;

			auto const dx = 0.5f*(neighbours[0] - neighbours[2])/pixel_size;
			auto const dy = 0.5f*(neighbours[1] - neighbours[3])/pixel_size;
			terraformer::displacement const gradvec{dx, dy, 0.0f};
			auto const grad_squared = norm_squared(gradvec);
			max_grad_out = std::max(grad_squared, max_grad_out);
			auto const xi = 1.0f;
			if(grad_squared > xi*xi || dz_max > xi*pixel_size)
			{
				auto const sample_from = terraformer::location{
					static_cast<float>(x),
					static_cast<float>(y),
					0.0f
				} + gradvec/std::sqrt(grad_squared);

				auto const uphill_value = std::max(
					grad_squared >0.0f? interp(input, sample_from[0], sample_from[1], terraformer::clamp_at_boundary{}): 0.0f,
					std::max(heighest_neighbour, current_value)
				);

				auto const required_value = uphill_value - xi*pixel_size;
				auto const error = std::max(required_value - current_value, 0.0f);
				auto const output_val = current_value + error/8.0f;
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

void grad_check(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	float pixel_size
)
{
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
			output(x, y) = std::sqrt(norm_squared(gradvec));
		}
	}
}

int main()
{
#if 1
	terraformer::grayscale_image buffer_a{2048, 2048};
	buffer_a(1024, 1024) = 1024.0f;
#else
	auto buffer_a = load(
		terraformer::empty<terraformer::grayscale_image>{},
		"experiments/rolling_hills.exr"
	);
#endif

	auto buffer_b = terraformer::create_with_same_size(buffer_a.pixels());

	auto pixels_a = buffer_a.pixels();
	auto pixels_b = buffer_b.pixels();

	for(size_t k = 0; k != 8192; ++k)
	{
		auto ret = run_pass(pixels_b, pixels_a, 4.0f);
		if(k % 128 == 0)
		{ printf("%zu %.8g\n", k, std::sqrt(ret)); }
		if(ret <= 1.0f)
		{ break; }

		std::swap(pixels_a, pixels_b);
	}

	store(pixels_b, "/dev/shm/output.exr");
	grad_check(pixels_a, pixels_b, 4.0f);
	store(pixels_a, "/dev/shm/grad.exr");
}
