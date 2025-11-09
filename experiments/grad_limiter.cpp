//@	{"target":{"name":"grad_limiter.o"}}

#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/math_utils/boundary_sampling_policies.hpp"
#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/math_utils/interp.hpp"
#include <algorithm>
#include <geosimd/basic_vector.hpp>
#include <utility>

struct gradient_limiter_descriptor
{
	float gradient_threshold;
	float pixel_size;
};

float run_pass(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	gradient_limiter_descriptor params
)
{
	auto const max_gradient = params.gradient_threshold;
	auto const pixel_size = params.pixel_size;
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
			auto const ddx = 0.5f*(neighbours[0] - neighbours[2])/pixel_size;
			auto const ddy = 0.5f*(neighbours[1] - neighbours[3])/pixel_size;
			terraformer::displacement const gradvec{ddx, ddy, 0.0f};
			auto const grad_squared = norm_squared(gradvec);
			max_grad_out = std::max(grad_squared, max_grad_out);
			auto get_uphill_value = [&](){
				if(grad_squared > max_gradient*max_gradient)
				{
					auto const sample_from = terraformer::location{
						static_cast<float>(x),
						static_cast<float>(y),
						0.0f
					} + gradvec/std::sqrt(grad_squared);
					return interp(input, sample_from[0], sample_from[1], terraformer::clamp_at_boundary{});
				}
				else
				if(dz_max > max_gradient*pixel_size)
				{ return heighest_neighbour; }
				else
				{ return input(x, y); }
			};

			auto const required_value = get_uphill_value() - max_gradient*pixel_size;
			auto const error = std::min(current_value - required_value, 0.0f);
			output(x, y) = current_value - error;
		}
	}

	return max_grad_out;
}

template<class PixelBlendFactor>
struct laplace_limiter_descriptor
{
	PixelBlendFactor blend_factor;
};

template<class PixelMask>
void run_pass(
	terraformer::span_2d<float> output,
	terraformer::span_2d<float const> input,
	laplace_limiter_descriptor<PixelMask>&& params
)
{
	for(int32_t y = 0; y != static_cast<int32_t>(output.height()); ++y)
	{
		for(int32_t x = 0; x != static_cast<int32_t>(output.width()); ++x)
		{
			using clamp_tag = terraformer::span_2d_extents::clamp_tag;
			auto const filtered_val = 0.25f*(
					input(x + 1, y, clamp_tag{})
				+input(x - 1, y, clamp_tag{})
				+input(x, y + 1, clamp_tag{})
				+input(x, y - 1, clamp_tag{})
			);
			output(x, y) = std::lerp(input(x, y), filtered_val, params.blend_factor(x, y));
		}
	}
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

void laplace_check(
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
			auto const ddx2 = (
				  input(x + 1, y, clamp_tag{})
				- 2.0f*input(x, y, clamp_tag{})
				+ input(x - 1, y, clamp_tag{})
			)/(pixel_size*pixel_size);
			auto const ddy2 = (
				  input(x, y + 1, clamp_tag{})
				- 2.0f*input(x, y, clamp_tag{})
				+ input(x, y + 1, clamp_tag{})
			)/(pixel_size*pixel_size);
			output(x, y) = ddx2 + ddy2;
		}
	}
}

terraformer::grayscale_image set_max_gradient_to(
	terraformer::span_2d<float const> input,
	gradient_limiter_descriptor grad_limiter
)
{
	auto buffer_in = terraformer::grayscale_image{input};
	auto buffer_out = terraformer::create_with_same_size<float>(input);

	auto ret_prev = run_pass(buffer_out.pixels(), buffer_in.pixels(), grad_limiter);
	std::swap(buffer_in, buffer_out);
	while(true)
	{
		auto ret =  run_pass(buffer_out.pixels(), buffer_in.pixels(), grad_limiter);;
		if(std::abs(ret - ret_prev) < 1.0e-4f)
		{ break; }
		ret_prev = ret;
		std::swap(buffer_in, buffer_out);
	}

	return buffer_out;
}


int main()
{
	auto const input = load(
		terraformer::empty<terraformer::grayscale_image>{},
		"experiments/rolling_hills.exr"
	);

	auto const after_grad_limiter = set_max_gradient_to(
		input.pixels(),
		gradient_limiter_descriptor{
			.gradient_threshold = 1.0f,
			.pixel_size = 8.0f
		}
	);
	store(after_grad_limiter.pixels(), "/dev/shm/after_grad_limiter.exr");

	auto buffer_out = terraformer::create_with_same_size<float>(after_grad_limiter.pixels());
	auto buffer_in = after_grad_limiter;
	for(size_t k = 0; k != 16384; ++k)
	{
		run_pass(buffer_out.pixels(), buffer_in.pixels(), laplace_limiter_descriptor{
			[
				pixels_in = input.pixels(),
				pixels_out = buffer_in.pixels(),
				pixel_size = 8.0f,
				g_in_max_2 = 0.25f,
				g_out_max_2 = 1.0f
			](int32_t x, int32_t y){
				using clamp_tag = terraformer::span_2d_extents::clamp_tag;
				auto const ddx_in = 0.5f*(pixels_in(x - 1, y, clamp_tag{}) - pixels_in(x + 1, y, clamp_tag{}))/pixel_size;
				auto const ddy_in = 0.5f*(pixels_in(x, y - 1, clamp_tag{}) - pixels_in(x, y + 1, clamp_tag{}))/pixel_size;
				auto const ddx_out = 0.5f*(pixels_out(x - 1, y, clamp_tag{}) - pixels_out(x + 1, y, clamp_tag{}))/pixel_size;
				auto const ddy_out = 0.5f*(pixels_out(x, y - 1, clamp_tag{}) - pixels_out(x, y + 1, clamp_tag{}))/pixel_size;
				return ddx_out*ddx_out + ddy_out*ddy_out < g_out_max_2 ? std::clamp((ddx_in*ddx_in + ddy_in*ddy_in)/g_in_max_2, 0.0f, 1.0f) : 0.0f;
			}
		});

		std::swap(buffer_out, buffer_in);
	}
	store(buffer_out.pixels(), "/dev/shm/after_laplace_limiter.exr");
}
