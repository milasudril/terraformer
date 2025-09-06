//@	{"target":{"name":"erode.o"}}

#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/interp.hpp"
#include <algorithm>
#include <random>

terraformer::grayscale_image erode(
	terraformer::span_2d<float const> input,
	terraformer::span_2d<float const> noise
)
{
	terraformer::grayscale_image ret{input.width(), input.height()};
	using clamp_tag = terraformer::span_2d_extents::clamp_tag;
	auto maxval = 0.0f;
	std::uniform_real_distribution noise_displacement{-256.0f, 256.0f};

	for(int32_t y = 0; y != static_cast<int32_t>(input.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(input.width()); ++x)
		{
			auto const ddx_input = input(x + 1, y, clamp_tag{}) - input(x - 1, y, clamp_tag{});
			auto const ddy_input = input(x, y + 1, clamp_tag{}) - input(x, y - 1, clamp_tag{});
			terraformer::displacement grad_z{ddx_input, ddy_input, 0.0f};
			auto const grad_size = norm(grad_z);
			terraformer::location current_loc{static_cast<float>(x), static_cast<float>(y), 0.0f};

			auto const input_val = input(x, y);

			auto const sample_from = current_loc - grad_z/grad_size;
			auto const downhill_value = terraformer::interp(
				input,
				sample_from[0],
				sample_from[1],
				terraformer::clamp_at_boundary{}
			);

			std::array vals{
				input_val,
				input(x + 1, y, clamp_tag{}),
				input(x - 1, y, clamp_tag{}),
				input(x, y + 1, clamp_tag{}),
				input(x, y - 1, clamp_tag{}),
			};

			auto const noise_val = noise(x ,y);

			auto const minval = (downhill_value >= input_val || (ddx_input == 0.0f && ddy_input == 0.0f))?
				*std::ranges::min_element(vals) : downhill_value;

			auto const val = std::lerp(
				input_val,
				minval,
				input_val*noise_val*noise_val/3500.0f
			);
			maxval = std::max(val, maxval);

			ret(x, y) = val;
		}
	}

	for(int32_t y = 0; y != static_cast<int32_t>(input.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(input.width()); ++x)
		{
			ret(x, y) = 3500.0f*ret(x, y)/maxval;
		}
	}
	return ret;
}

terraformer::grayscale_image make_noise(uint32_t width, uint32_t height, terraformer::random_generator& rng)
{
	terraformer::grayscale_image raw_noise{width, height};
	std::uniform_real_distribution input_intensity{0.0f, 1.0f};
	for(uint32_t y = 0;  y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{ raw_noise(x, y) = input_intensity(rng); }
	}

	terraformer::grayscale_image filtered_noise{width, height};

	constexpr auto kernel_width = 5;
	constexpr auto kernel_height = 5;

	std::array<std::array<float, kernel_width>, kernel_width> kernel{
		std::array{0.0f, 1.0f, 1.0f, 1.0f, 0.0f},
		std::array{1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
		std::array{1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
		std::array{1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
		std::array{0.0f, 1.0f, 1.0f, 1.0f, 0.0f}
	};

	auto maxval = 0.0f;

	for(uint32_t y = 0; y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{
			auto sum = 0.0f;
			for(int k = 0; k != kernel_height; ++k)
			{
				for(int l = 0; l != kernel_width; ++l)
				{
					sum += raw_noise(
						(x + width + l - kernel_width/2)%width,
						(y + height + k - kernel_height/2)%height
					)*kernel[k][l];
				}
			}

			maxval = std::max(maxval, sum);
			filtered_noise(x, y) = sum;
		}
	}

	for(uint32_t y = 0; y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{ filtered_noise(x, y) /= maxval; }
	}

	return filtered_noise;
}

void accumulate(terraformer::span_2d<float> output, terraformer::span_2d<float const> input, float factor)
{
	auto const width = output.width();
	auto const height = output.height();
	for(uint32_t y = 0; y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{ output(x, y) = (1.0f - factor)*output(x, y) + factor*input(x, y); }
	}
}

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		return -1;
	}

	terraformer::random_generator rng;
	auto input = load(terraformer::empty<terraformer::grayscale_image>{}, argv[1]);
	auto noise = make_noise(input.width(), input.height(), rng);

	for(size_t k = 0; k != 1024; ++k)
	{
		input = erode(input, noise);
		auto const new_noise = make_noise(input.width(), input.height(), rng);
		accumulate(noise.pixels(), new_noise.pixels(), 0.25f);

		printf("%zu\n", k);
	}

	store(input.pixels(), static_cast<char const*>(argv[2]));
	return 0;
}