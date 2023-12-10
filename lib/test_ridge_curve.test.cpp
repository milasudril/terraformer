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

#if 0
	auto const x_intercepts = terraformer::find_zeros(offsets);
	auto value = (offsets[0] >= 0.0f)? 2560.0f: -2560.0f;
	size_t l = 0;
	for(size_t k = 0; k != std::size(offsets);++k)
	{
		if(l != std::size(x_intercepts))
		{
			if(k == x_intercepts[l])
			{
	//			printf("%zu\n", x_intercepts[l]);
				++l;
				value = -value;
			}
		}
		printf("%zu %.8g %.8g\n", k, offsets[k], value);
	}

	auto const points = displace(curve, terraformer::displacement_profile{.offsets = offsets, .sample_period = pixel_size}, terraformer::displacement{0.0f, 0.0f, -1.0f});
	std::vector<float> branch_prob(std::size(points));
	for(size_t k = 1; k != std::size(points) - 1; ++k)
	{
		auto const x = static_cast<float>(k)*pixel_size;
		auto const dx_coarse = curve_desc.wavelength/(1.35f*8.0f);
		auto const a_coarse = interp(points, (x - dx_coarse)/pixel_size, terraformer::clamp_at_boundary{});
		auto const b_coarse = interp(points, x/pixel_size, terraformer::clamp_at_boundary{});
		auto const c_coarse = interp(points, (x + dx_coarse)/pixel_size, terraformer::clamp_at_boundary{});
		terraformer::direction const dir_1_coarse{b_coarse - a_coarse};
		terraformer::direction const dir_2_coarse{c_coarse - b_coarse};
		terraformer::direction const tangent_points_coarse{c_coarse - a_coarse};
//		auto const normal_coarse = terraformer::curve_vertex_normal_from_curvature(a_coarse, b_coarse, c_coarse);

		auto const a_fine = points[k - 1];
		auto const b_fine = points[k];
		auto const c_fine = points[k +1 ];
		terraformer::direction const dir_1_fine{b_fine - a_fine};
		terraformer::direction const dir_2_fine{c_fine - b_fine};
		terraformer::direction const tangent_points_fine{c_fine - a_fine};
		auto const c1_fine = curve[k - 1];
		auto const c2_fine = curve[k + 1];
		terraformer::direction const tangent_curve_fine{c2_fine - c1_fine};
	//	auto const normal_fine = terraformer::curve_vertex_normal_from_curvature(a_fine, b_fine, c_fine);

		branch_prob[k] = std::acos(inner_product(dir_1_coarse, dir_2_coarse)); //std::acos(inner_product(dir_1_fine, dir_2_fine)));
		//	*(std::abs(inner_product(tangent_points_fine, tangent_curve_fine)) > 0.5f? 1.0f : 0.0f)
		//	*(std::abs(inner_product(tangent_points_coarse, tangent_curve_fine)) > std::sqrt(0.5f)? 1.0f : 0.0f)
		//	*(inner_product(normal_fine, b_fine - curve[k]) > 0.0f? 1.0f: 0.0f)
		//	*(inner_product(normal_coarse, b_fine - curve[k]) > 0.0f ? 1.0f: 0.0f);
	}

	{
		auto const maxval = *std::ranges::max_element(branch_prob);
		for(size_t k = 0; k != std::size(branch_prob); ++k)
		{	branch_prob[k] /= maxval;}
	}
	for(size_t k = 0; k != std::size(points); ++k)
	{
		auto const do_branch = branch_prob[k] > 0.5f; //std::bernoulli_distribution{branch_prob[k]}(rng);
		printf("%.8g %.8g %.8g %d\n", points[k][0], points[k][1], branch_prob[k], do_branch);
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
#endif
}