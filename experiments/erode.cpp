//@	{"target":{"name":"erode.o"}}

#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/interp.hpp"
#include <algorithm>
#include <random>

void amplify(terraformer::span_2d<float> input, float gain)
{
	auto const width = input.width();
	auto const height = input.height();
	for(uint32_t y = 0; y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{
			input(x, y) *= gain;
		}
	}
}

[[nodiscard]] float erode(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	terraformer::span_2d<float const> noise,
	float peak_elevation
)
{
	terraformer::grayscale_image ret{input.width(), input.height()};
	using clamp_tag = terraformer::span_2d_extents::clamp_tag;
	auto maxval = 0.0f;

	for(int32_t y = 0; y != static_cast<int32_t>(input.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(input.width()); ++x)
		{
			auto const ddx_input = input(x + 1, y, clamp_tag{}) - input(x - 1, y, clamp_tag{});
			auto const ddy_input = input(x, y + 1, clamp_tag{}) - input(x, y - 1, clamp_tag{});
			terraformer::displacement grad_z{ddx_input, ddy_input, 0.0f};
			auto const grad_size = norm(grad_z);
			if(grad_size  == 0.0f)
			{ continue; }


			terraformer::location current_loc{static_cast<float>(x), static_cast<float>(y), 0.0f};

			auto const input_val = input(x, y);

			auto const sample_from = current_loc - grad_z/grad_size;
			auto const downhill_value = terraformer::interp(
				input,
				sample_from[0],
				sample_from[1],
				terraformer::clamp_at_boundary{}
			);

			auto const noise_val = noise(x ,y);
			auto const minval = std::min(input_val, downhill_value);

			auto const val = std::lerp(
				input_val,
				minval,
				input_val*noise_val*noise_val/peak_elevation
			);
			maxval = std::max(val, maxval);

			output(x, y) = val;
		}
	}
	return maxval;
}

void make_white_noise(terraformer::span_2d<float> output, terraformer::random_generator& rng)
{
	auto const width = output.width();
	auto const height = output.height();

	std::uniform_real_distribution input_intensity{0.0f, 1.0f};
	for(uint32_t y = 0;  y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{ output(x, y) = input_intensity(rng); }
	}
}

[[nodiscard]] float apply_lowpass_filter(terraformer::span_2d<float> output, terraformer::span_2d<float const> input)
{
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
	auto const width = output.width();
	auto const height = output.height();

	for(uint32_t y = 0; y != height; ++y)
	{
		for(uint32_t x = 0; x != width; ++x)
		{
			auto sum = 0.0f;
			for(int k = 0; k != kernel_height; ++k)
			{
				for(int l = 0; l != kernel_width; ++l)
				{
					sum += input(
						(x + width + l - kernel_width/2)%width,
						(y + height + k - kernel_height/2)%height
					)*kernel[k][l];
				}
			}

			maxval = std::max(maxval, sum);
			output(x, y) = sum;
		}
	}
	return maxval;
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
	auto buffer_a = load(terraformer::empty<terraformer::grayscale_image>{}, argv[1]);
	auto buffer_b = buffer_a;

	auto input = buffer_a.pixels();
	auto output = buffer_b.pixels();

	auto white_noise_buffer = buffer_a;
	auto filtered_noise_buffer = buffer_a;
	auto accumulated_noise = buffer_a;

	make_white_noise(white_noise_buffer.pixels(), rng);
	auto maxval = apply_lowpass_filter(accumulated_noise.pixels(), white_noise_buffer.pixels());
	amplify(accumulated_noise.pixels(), 1.0f/maxval);

	for(size_t k = 0; k != 1024; ++k)
	{
		maxval = erode(output, input, accumulated_noise.pixels(), 3500.0f);
		amplify(output, 3500.0f/maxval);
		std::swap(output, input);
		make_white_noise(white_noise_buffer.pixels(), rng);
		maxval = apply_lowpass_filter(filtered_noise_buffer.pixels(), white_noise_buffer.pixels());
		amplify(filtered_noise_buffer.pixels(), 1.0f/maxval);
		accumulate(accumulated_noise.pixels(), filtered_noise_buffer.pixels(), 0.25f);
		printf("%zu\n", k);
	}

	store(output, static_cast<char const*>(argv[2]));
	return 0;
}