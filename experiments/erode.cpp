//@	{"target":{"name":"erode.o"}}

#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/interp.hpp"
#include <algorithm>
#include <random>

#if 0
terraformer::grayscale_image erode(
	terraformer::span_2d<float const> input,
	terraformer::span_2d<float const> noise
)
{
	auto const dx = 49152.0f/static_cast<float>(input.width());
	auto const dy = 49152.0f/static_cast<float>(input.height());
	auto const dt = 4.0f;
	terraformer::grayscale_image ret{input.width(), input.height()};
	using clamp_tag = terraformer::span_2d_extents::clamp_tag;

	std::uniform_real_distribution U{0.0f, 1.0f};

	for(int32_t y = 0; y != static_cast<int32_t>(input.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(input.width()); ++x)
		{
			auto const ddx_input = 0.5f*(input(x + 1, y, clamp_tag{}) - input(x - 1, y, clamp_tag{}))/dx;
			auto const ddy_input = 0.5f*(input(x, y + 1, clamp_tag{}) - input(x, y - 1, clamp_tag{}))/dy;
			auto const ddx_noise = 0.5f*(noise(x + 1, y, clamp_tag{}) - noise(x - 1, y, clamp_tag{}))/dx;
			auto const ddy_noise = 0.5f*(noise(x, y + 1, clamp_tag{}) - noise(x, y - 1, clamp_tag{}))/dy;

			auto const d2dx2_input = (
				  input(x + 1, y, clamp_tag{})
				- 2.0f*input(x, y)
				+ input(x - 1, y, clamp_tag{}))/
				(dx*dx);

			auto const d2dy2_input = (
				  input(x, y + 1, clamp_tag{})
				- 2.0f*input(x, y)
				+ input(x, y - 1, clamp_tag{}))/
				(dy*dy);

			terraformer::displacement const grad_z{ddx_input, ddy_input, 0.0f};
			terraformer::displacement const grad_noise{ddx_noise, ddy_noise, 0.0f};
			auto const laplace_input = d2dx2_input + d2dy2_input;

			auto const ddt_z = 0.0f*laplace_input*noise(x, y) + inner_product(grad_z, 16.0f*grad_noise);

			ret(x, y) = input(x, y) + dt*ddt_z;
		}
	}

	return ret;
}
#else

terraformer::grayscale_image erode(
	terraformer::span_2d<float const> input,
	terraformer::span_2d<float const> noise,
	terraformer::random_generator& rng
)
{
	terraformer::grayscale_image ret{input.width(), input.height()};
	using clamp_tag = terraformer::span_2d_extents::clamp_tag;
	auto maxval = 0.0f;
	auto max_grad = 0.0f;
	auto const dx = 15.0f;
	auto const dy = 15.0f;
	std::uniform_real_distribution noise_displacement{-256.0f, 256.0f};
	for(int32_t y = 0; y != static_cast<int32_t>(input.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(input.width()); ++x)
		{
			auto const ddx_input = 0.5f*(input(x + 1, y, clamp_tag{}) - input(x - 1, y, clamp_tag{}))/dx;
			auto const ddy_input = 0.5f*(input(x, y + 1, clamp_tag{}) - input(x, y - 1, clamp_tag{}))/dy;
			terraformer::displacement grad_z{ddx_input, ddy_input, 0.0f};
			auto const grad_size = norm(grad_z);
			max_grad = std::max(max_grad, grad_size);

			terraformer::location current_loc{static_cast<float>(x), static_cast<float>(y), 0.0f};
			auto const sample_from = current_loc - grad_z/grad_size;

			auto const input_val = input(x, y);

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

			auto const noise_val =interp(
				noise,
				0.5f*static_cast<float>(x) + noise_displacement(rng)/dx,
				0.5f*static_cast<float>(y) + noise_displacement(rng)/dy,
				terraformer::wrap_around_at_boundary{}
			);

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

	printf("%.8g\n", max_grad);
	return ret;
}

#endif

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		return -1;
	}

	auto input = load(terraformer::empty<terraformer::grayscale_image>{}, argv[1]);
	auto const noise = load(terraformer::empty<terraformer::grayscale_image>{}, argv[2]);

	terraformer::random_generator rng;

	for(size_t k = 0; k != 4096; ++k)
	{
		printf("%zu  ", k);
		input = erode(input.pixels(), noise, rng);
	}

	store(input.pixels(), static_cast<char const*>(argv[3]));

	return 0;
}