//@ {"target":{"name":"test_ridge_curve"}}

#include "./ridge_curve.hpp"
#include "./curve_displace.hpp"

#include "lib/pixel_store/image_io.hpp"

namespace terraformer
{
	std::vector<location> make_point_array(location start_loc, size_t count, float dx)
	{
		std::vector<location> ret(count);
		for(size_t k = 0; k != count; ++k)
		{
			// Note: Add a small disturbance to y coordinate to avoid singular values when computing
			//       the curve normal
			ret[k] = start_loc
				+ displacement{static_cast<float>(k)*dx, (static_cast<float>(k%2) - 0.5f)*dx/1024.0f, 0.0f};
		}
		return ret;
	}
}

int main()
{
	terraformer::ridge_curve_description curve_desc{
		.amplitude = terraformer::horizontal_amplitude{2560.0f},
		.wavelength = terraformer::domain_length{12384.0f},
		.damping = std::sqrt(0.5f),
		.flip_direction = false,
		.invert_displacement = false
	};

	constexpr auto pixel_size = 48.0f;
	terraformer::random_generator rng;
	auto const pixel_count = static_cast<size_t>(49152.0f/pixel_size);
	auto const offsets = generate(
		curve_desc,
		rng,
		pixel_count,
		pixel_size,
		// TODO: Should use a randomized value here
		static_cast<size_t>(24.0f*curve_desc.wavelength/(curve_desc.damping*pixel_size)));
	auto const ridge_loc = 24576.0f;
	auto const curve = terraformer::make_point_array(terraformer::location{0.0f, ridge_loc, 0.0f}, pixel_count, pixel_size);
	auto const points = displace(curve, terraformer::displacement_profile{.offsets = offsets, .sample_period = pixel_size}, terraformer::displacement{0.0f, 0.0f, -1.0f});

	for(size_t k = 1; k != std::size(points) - 1; ++k)
	{
		auto const dir_1 = terraformer::direction{points[k] - points[k - 1]};
		auto const dir_2 = terraformer::direction{points[k+1] - points[k]};
		auto const theta = std::acos(inner_product(dir_1, dir_2));
		printf("%.8g %.8g %.8g\n", points[k][0], points[k][1], theta);
	}

	terraformer::grayscale_image potential{pixel_count, pixel_count};
	for(uint32_t y = 0; y != potential.height(); ++y)
	{
		for(uint32_t x = 0; x != potential.width(); ++x)
		{
			terraformer::location const loc_xy{pixel_size*static_cast<float>(x), pixel_size*static_cast<float>(y), 0.0f};
			auto sum = std::accumulate(std::begin(points), std::end(points), 0.0f, [loc_xy](auto const sum, auto const point) {
				auto const d = terraformer::distance_xy(loc_xy, point);
				auto const d_min = 1.0f*pixel_size;
				return sum + 1.0f*(d<d_min? 1.0f : (d_min*d_min)/(d*d));
			});

			potential(x, y) = sum;
		}
	}

	store(potential, "test.exr");
}