//@ {"target":{"name":"test_ridge_curve"}}

#include "./ridge_curve.hpp"
#include "./boundary_sampling_policies.hpp"
#include "./ridge_tree_branch.hpp"

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

	std::vector<std::vector<location>>
	generate_branches(array_tuple<location, direction> const& seeds, span_2d<float const> potential, float pixel_size,
	std::vector<std::vector<location>>&& existing_branches = std::vector<std::vector<location>>{})
	{
		auto const points = seeds.get<0>();
		auto const normals = seeds.get<1>();
		for(size_t k = 0; k != std::size(seeds); ++k)
		{
			std::vector curve{points[k]};
			auto loc = points[k] + pixel_size*normals[k];
			for(size_t l = 0; l != 1024; ++l)
			{
				curve.push_back(loc);
				loc -= pixel_size*direction{
					grad(
						potential,
						loc[0]/pixel_size,
						loc[1]/pixel_size,
						1.0f,
						clamp_at_boundary{}
					)
				};

				if(loc[0] <= 2.0f
					|| loc[1] <= 2.0f
					|| loc[0] >= pixel_size*static_cast<float>(potential.width() - 2)
					|| loc[1] >= pixel_size*static_cast<float>(potential.height() - 2))
				{ break; }
			}
			existing_branches.push_back(std::move(curve));
		}

		return existing_branches;
	}

	std::vector<std::vector<location>>
	generate_branches(std::span<ridge_tree_branch const> trees,
		span_2d<float const> potential,
		float pixel_size
	)
	{
		auto branches = generate_branches(
			trees[0].left_seeds(),
			potential,
			pixel_size
		);

		for(size_t k = 1; k != std::size(trees); ++k)
		{
			branches = generate_branches(
				trees[k - 1].right_seeds(),
				potential,
				pixel_size,
				std::move(branches)
			);

			branches = generate_branches(
				trees[k].left_seeds(),
				potential,
				pixel_size,
				std::move(branches)
			);
		}

		return branches;
	}
}

int main()
{
	terraformer::ridge_curve_description curve_desc{
		.amplitude = terraformer::horizontal_amplitude{3096.0f},
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
		1024.0f);
	auto const ridge_loc = 24576.0f;
	auto const curve = terraformer::make_point_array(terraformer::location{0.0f, ridge_loc, 0.0f}, pixel_count, pixel_size);

	terraformer::ridge_tree_branch root{
		curve,
		terraformer::displacement_profile{
			.offsets = offsets,
			.sample_period = pixel_size
		}
	};

	terraformer::grayscale_image potential{pixel_count, pixel_count};
	{
		auto const& root_curve = root.curve();
		auto const points = root_curve.get<0>();
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
	}

	auto const branches = generate_branches(
		root.right_seeds(),
		potential,
		pixel_size,
		generate_branches(root.left_seeds(), potential, pixel_size)
	);

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