//@ {"target":{"name":"test_ridge_curve"}}

#include "./ridge_curve.hpp"
#include "./curve_displace.hpp"
#include "./find_zeros.hpp"
#include "./boundary_sampling_policies.hpp"
#include "./interleave.hpp"

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


	auto const points = displace(curve, terraformer::displacement_profile{.offsets = offsets, .sample_period = pixel_size}, terraformer::displacement{0.0f, 0.0f, -1.0f});
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

	std::vector<size_t> branch_at;
	{
		auto const x_intercepts = terraformer::find_zeros(offsets);
		auto side = (offsets[0] >= 0.0f)? 1.0f: -1.0f;
		size_t l = 0;
		if(l != std::size(x_intercepts) && x_intercepts[l] == 0)
		{
			++l;
			side = -side;
		}

		float max_offset = 0.0f;
		std::optional<size_t> selected_branch_point;
		std::vector<size_t> branch_at_tmp;
		for(size_t k = 1; k != std::size(offsets) - 1;++k)
		{
			if(l != std::size(x_intercepts) && k == x_intercepts[l])
			{
				if(selected_branch_point.has_value())
				{ branch_at_tmp.push_back(*selected_branch_point); }
				max_offset = 0.0f;
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

			if(visible && std::abs(y) > max_offset && side*y > side*offsets[k - 1] && side*y < side*offsets[k + 1])
			{
				max_offset = std::abs(y);
				selected_branch_point = k;
			}
		}

		if(selected_branch_point.has_value())
		{ branch_at_tmp.push_back(*selected_branch_point); }

		branch_at = terraformer::interleave(std::span{std::as_const(branch_at_tmp)});
	}

	std::vector<std::vector<terraformer::location>> branches;
	for(size_t k = 0; k != std::size(branch_at); ++k)
	{
		auto const index = branch_at[k];
		auto const points_a = points[index - 1];
		auto const points_b = points[index];
		auto const points_c = points[index + 1];
		auto const normal = terraformer::curve_vertex_normal_from_curvature(points_a, points_b, points_c);

		std::vector curve{points_b};
		auto loc = points_b + pixel_size*normal;
		for(size_t k = 0; k != 1024; ++k)
		{
			curve.push_back(loc);
			loc -= pixel_size*terraformer::direction{
				grad(
					std::as_const(potential).pixels(),
					loc[0]/pixel_size,
					loc[1]/pixel_size,
					1.0f,
					terraformer::clamp_at_boundary{}
				)
			};

			if(loc[0] <= 2.0f
				|| loc[1] <= 2.0f
				|| loc[0] >= pixel_size*static_cast<float>(potential.width() - 2)
				|| loc[1] >= pixel_size*static_cast<float>(potential.height() - 2))
			{ break; }
		}
		branches.push_back(std::move(curve));
	}

	for(uint32_t y = 0; y != potential.height(); ++y)
	{
		for(uint32_t x = 0; x != potential.width(); ++x)
		{
			auto sum = 0.0f;
			for(size_t k = 0; k != std::size(branches); ++k)
			{
				auto const& points = branches[k];
				terraformer::location const loc_xy{pixel_size*static_cast<float>(x), pixel_size*static_cast<float>(y), 0.0f};
				sum += std::accumulate(std::begin(points), std::end(points), 0.0f, [loc_xy](auto const sum, auto const point) {
					auto const d = terraformer::distance_xy(loc_xy, point);
					auto const d_min = 1.0f*pixel_size;
					return sum + 1.0f*(d<d_min? 1.0f : (d_min*d_min)/(d*d));
				});
			}
			potential(x, y) += sum;
		}
	}
	store(potential, "test.exr");
}