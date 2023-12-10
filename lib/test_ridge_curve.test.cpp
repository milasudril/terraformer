//@ {"target":{"name":"test_ridge_curve"}}

#include "./ridge_curve.hpp"
#include "./curve_displace.hpp"
#include "./find_zeros.hpp"
#include "./boundary_sampling_policies.hpp"

#include "lib/pixel_store/image_io.hpp"

#include <random>

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
		.amplitude = terraformer::horizontal_amplitude{3072.0f},
		.wavelength = terraformer::domain_length{12384.0f*1.35f},
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

	auto const x_intercepts = terraformer::find_zeros(offsets);

	auto const points = displace(curve, terraformer::displacement_profile{.offsets = offsets, .sample_period = pixel_size}, terraformer::displacement{0.0f, 0.0f, -1.0f});

	auto side = (offsets[0] >= 0.0f)? 1.0f: -1.0f;
	size_t l = 0;
	if(l != std::size(x_intercepts) && x_intercepts[l] == 0)
	{
		++l;
		side = -side;
	}
	std::vector<float> branch_prob(std::size(points));
	auto branch_prob_tot = 0.0;
	for(size_t k = 1; k != std::size(offsets) - 1;++k)
	{
		if(l != std::size(x_intercepts) && k == x_intercepts[l])
		{
			++l;
			side = -side;
		}

		auto const y = offsets[k];
		auto const points_a = points[k - 1];
		auto const points_b = points[k];
		auto const points_c = points[k + 1];
		auto const points_normal = terraformer::curve_vertex_normal_from_projection(points_a, points_b, points_c, terraformer::displacement{0.0f, 0.0f, -1.0f});
		auto const points_ab = points_b - points_a;
		auto const side_of_curve = inner_product(points_ab, points_normal);
		auto const visible = (side*y > 0.0f ? 1.0f : 0.0f)*(side*side_of_curve > 0.0f ? 1.0f : 0.0f);

		branch_prob[k] = visible*y*y;
		branch_prob_tot += static_cast<double>(visible)*static_cast<double>(y)*static_cast<double>(y);
	}

	for(size_t k = 0; k != std::size(branch_prob); ++k)
	{ branch_prob[k] *= static_cast<float>(static_cast<double>(1.0)/branch_prob_tot); }

	for(size_t k = 0; k != std::size(points); ++k)
	{
		auto const do_branch = std::bernoulli_distribution{branch_prob[k]}(rng);
		printf("%.8g %.8g %.8g %d\n", points[k][0], points[k][1], branch_prob[k], do_branch);
	}
#if 0

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
#endif
}