//@	{"target":{"name":"grad_limiter.o"}}

#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include <algorithm>

float run_pass(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	float pixel_size
)
{
	auto max_grad_out = 0.0f;
	for(uint32_t y = 1; y != output.height() - 1; ++y)
	{
		for(uint32_t x = 1; x != output.width() - 1; ++x)
		{
			std::array const neighbours{
				input(x + 1, y),
				input(x - 1 ,y),
				input(x, y + 1),
				input(x, y - 1)
			};

			auto const heighest_neigbour = *std::ranges::max_element(neighbours);
			auto const dz = heighest_neigbour - input(x, y);
			if(dz > pixel_size)
			{
				auto const required_value = (heighest_neigbour - pixel_size)*(1.0f + 1.0f/1024.0f);
				auto const current_value = input(x, y);
				auto const error = required_value - current_value;
				auto const output_val = current_value + error/32.0f;
				output(x, y) = output_val;
				max_grad_out = std::max(std::abs(heighest_neigbour - output_val), max_grad_out);
			}
			else
			{
				auto const output_val = input(x, y);
				max_grad_out = std::max(std::abs(heighest_neigbour - output_val), max_grad_out);
				output(x, y) = output_val;
			}
		}
	}

	return max_grad_out/pixel_size;
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

	for(size_t k = 0; k != 16384; ++k)
	{
		auto ret = run_pass(pixels_b, pixels_a, 8);
		if(k % 128 == 0)
		{ printf("%.8g\n", ret); }
		if(ret <= 1.0f)
		{ break; }

		std::swap(pixels_a, pixels_b);
	}

	store(pixels_b, "/dev/shm/output.exr");
}