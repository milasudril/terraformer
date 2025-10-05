//@	{"target":{"name":"ridge_tree_branch_seed_sequence.test"}}

#include "./ridge_tree_branch_seed_sequence.hpp"

#include "lib/curve_tools/displace.hpp"
#include "lib/common/rng.hpp"

#include <testfwk/testfwk.hpp>
#include <random>

TESTCASE(terraformer_ridge_tree_branch_seed_sequence_pair_plus_minus_plus)
{
	constexpr size_t count = 513;
	std::array<terraformer::location, count> base_curve;
	{
		constexpr auto dx = 1.0f/static_cast<float>(std::size(base_curve) - 1);
		for(size_t k = 0; k != std::size(base_curve); ++k)
		{
			auto const x = dx*static_cast<float>(k);
			auto const y = x*(1.0f - x);
			base_curve[k] = terraformer::location{x, y, 0.0f};
		}
	}

	std::array<float, count> offsets;
	{
		constexpr auto dx = 1.0f/static_cast<float>(std::size(base_curve) - 1);
		for(size_t k = 0; k != std::size(offsets); ++k)
		{
			auto const x = dx*static_cast<float>(k) - 0.5f;
			auto const y = -0.125f*std::cos(2.0f*std::numbers::pi_v<float>*x*1.25f);
			offsets[k] = y;
		}
	}

	constexpr auto dx = 1.0f/static_cast<float>(count - 1);
	auto const res = terraformer::collect_ridge_tree_branch_seeds(
		terraformer::displace_xy(
			base_curve,
			terraformer::displacement_profile{
				.offsets = offsets,
				.sample_period = 1.15f*dx
			}
		),
		terraformer::ridge_tree_branch_seed_collection_descriptor{
			.start_branches = terraformer::ridge_tree_brach_seed_sequence_boundary_point_descriptor{},
			.end_brancehs = terraformer::ridge_tree_brach_seed_sequence_boundary_point_descriptor{
				.branch_count = 2,
				.spread_angle = geosimd::turns{0.5f}
			}
		}
	);

	EXPECT_EQ(std::size(res.left).get(), 3);
	EXPECT_EQ(std::size(res.right).get(), 2);
}

TESTCASE(terraformer_ridge_tree_branch_seed_sequence_pair_minus_plus_minus)
{
	constexpr size_t count = 513;
	std::array<terraformer::location, count> base_curve;
	{
		constexpr auto dx = 1.0f/static_cast<float>(std::size(base_curve) - 1);
		for(size_t k = 0; k != std::size(base_curve); ++k)
		{
			auto const x = dx*static_cast<float>(k);
			auto const y = x*(1.0f - x);
			base_curve[k] = terraformer::location{x, y, 0.0f};
		}
	}

	std::array<float, count> offsets;
	{
		constexpr auto dx = 1.0f/static_cast<float>(std::size(base_curve) - 1);
		for(size_t k = 0; k != std::size(offsets); ++k)
		{
			auto const x = dx*static_cast<float>(k) - 0.5f;
			auto const y = 0.125f*std::cos(2.0f*std::numbers::pi_v<float>*x*1.25f);
			offsets[k] = y;
		}
	}

	constexpr auto dx = 1.0f/static_cast<float>(count - 1);

	auto const res = terraformer::collect_ridge_tree_branch_seeds(
		terraformer::displace_xy(
			base_curve,
			terraformer::displacement_profile{
				.offsets = offsets,
				.sample_period = 1.15f*dx
			}
		),
		terraformer::ridge_tree_branch_seed_collection_descriptor{
			.start_branches = terraformer::ridge_tree_brach_seed_sequence_boundary_point_descriptor{},
			.end_brancehs = terraformer::ridge_tree_brach_seed_sequence_boundary_point_descriptor{
				.branch_count = 2,
				.spread_angle = geosimd::turns{0.5f}
			}
		}
	);

	EXPECT_EQ(std::size(res.left).get(), 2);
	EXPECT_EQ(std::size(res.right).get(), 3);
}

TESTCASE(terraformer_ridge_tree_branch_seed_sequence_pair_plus_minus_plus_minus)
{
	constexpr size_t count = 513;
	std::array<terraformer::location, count> base_curve;
	{
		constexpr auto dx = 1.0f/static_cast<float>(std::size(base_curve) - 1);
		for(size_t k = 0; k != std::size(base_curve); ++k)
		{
			auto const x = dx*static_cast<float>(k);
			auto const y = x*(1.0f - x);
			base_curve[k] = terraformer::location{x, y, 0.0f};
		}
	}

	std::array<float, count> offsets;
	{
		constexpr auto dx = 1.0f/static_cast<float>(std::size(base_curve) - 1);
		for(size_t k = 0; k != std::size(offsets); ++k)
		{
			auto const x = dx*static_cast<float>(k) - 0.5f;
			auto const y = 0.125f*std::sin(3.0f*std::numbers::pi_v<float>*x*1.25f);
			offsets[k] = y;
		}
	}
	constexpr auto dx = 1.0f/static_cast<float>(count - 1);

	auto const res = terraformer::collect_ridge_tree_branch_seeds(
		terraformer::displace_xy(
			base_curve,
			terraformer::displacement_profile{
				.offsets = offsets,
				.sample_period = 1.15f*dx
			}
		),
		terraformer::ridge_tree_branch_seed_collection_descriptor{
			.start_branches = terraformer::ridge_tree_brach_seed_sequence_boundary_point_descriptor{},
			.end_brancehs = terraformer::ridge_tree_brach_seed_sequence_boundary_point_descriptor{
				.branch_count = 2,
				.spread_angle = geosimd::turns{0.5f}
			}
		}
	);

	EXPECT_EQ(std::size(res.left).get(), 3);
	EXPECT_LT(
		distance(res.left.get<0>().front(), (terraformer::location{0.17f, -0.015f, 0.0f})),
		0.05f
	);
	EXPECT_LT(res.left.get<1>().front()[1], 0.0f);

	EXPECT_EQ(std::size(res.right).get(), 3);
	EXPECT_LT(
		distance(res.right.get<0>()[res.right.element_indices().front() + 1], (terraformer::location{1.0f, 0.18f, 0.0f})),
		0.05f
	);
	EXPECT_GT(res.right.get<1>()[res.right.element_indices().front() + 1][1], 0.0f);
}

#if 0
// TODO
TESTCASE(terraformer_ridge_tree_branch_seed_sequence_pair_minus_plus_minus_plus)
{
	constexpr size_t count = 513;
	std::array<terraformer::location, count> base_curve;
	{
		constexpr auto dx = 1.0f/static_cast<float>(std::size(base_curve) - 1);
		for(size_t k = 0; k != std::size(base_curve); ++k)
		{
			auto const x = dx*static_cast<float>(k);
			auto const y = x*(1.0f - x);
			base_curve[k] = terraformer::location{x, y, 0.0f};
		}
	}

	std::array<float, count> offsets;
	{
		constexpr auto dx = 1.0f/static_cast<float>(std::size(base_curve) - 1);
		for(size_t k = 0; k != std::size(offsets); ++k)
		{
			auto const x = dx*static_cast<float>(k) - 0.5f;
			auto const y = -0.125f*std::sin(3.0f*std::numbers::pi_v<float>*x*1.25f);
			offsets[k] = y;
		}
	}
	constexpr auto dx = 1.0f/static_cast<float>(count - 1);

	auto const res = terraformer::collect_ridge_tree_branch_seeds(
		terraformer::displace_xy(
			base_curve,
			terraformer::displacement_profile{
				.offsets = offsets,
				.sample_period = 1.15f*dx
			}
		)
	);

	EXPECT_EQ(std::size(res.left).get(), 2);
	EXPECT_EQ(std::size(res.right).get(), 2);
}

TESTCASE(terraformer_ridge_tree_branch_seed_sequence_pair_random_data)
{
	std::array<float, 387> offsets{};
	{
		auto input_file = fopen("testdata/random_curve.dat", "rb");
		REQUIRE_NE(input_file, nullptr);
		auto const res = fread(std::data(offsets), sizeof(float), std::size(offsets), input_file);
		EXPECT_EQ(res, std::size(offsets));
		fclose(input_file);
	}

	std::array<terraformer::location, 387> base_curve{};
	{
		auto input_file = fopen("testdata/basecurve.dat", "rb");
		REQUIRE_NE(input_file, nullptr);
		auto const res = fread(std::data(base_curve), sizeof(terraformer::location), std::size(base_curve), input_file);
		EXPECT_EQ(res, std::size(base_curve));
		fclose(input_file);
	}

	terraformer::random_generator rng;
	std::uniform_real_distribution U{-1.0f/1024.0f, 1.0f/1024.0f};

	for(size_t k = 1; k != std::size(base_curve); ++k)
	{
		auto const d = distance(base_curve[k], base_curve[k - 1]);
		base_curve[k] += d*terraformer::displacement{U(rng), U(rng), 0.0f*U(rng)};
	}
	constexpr auto pixel_size = 32.0f;

	auto const res = terraformer::collect_ridge_tree_branch_seeds(
		terraformer::displace_xy(
			base_curve,
			terraformer::displacement_profile{
				.offsets = offsets,
				.sample_period = pixel_size
			}
		)
	);

	EXPECT_EQ(std::size(res.left).get(), 4);
	EXPECT_EQ(std::size(res.right).get(), 4);
}

TESTCASE(terraformer_ridge_tree_branch_seed_sequence_pair_random_data_2)
{
	std::array<float, 387> offsets{};
	{
		auto input_file = fopen("testdata/random_curve_2.dat", "rb");
		REQUIRE_NE(input_file, nullptr);
		auto const res = fread(std::data(offsets), sizeof(float), std::size(offsets), input_file);
		EXPECT_EQ(res, std::size(offsets));
		fclose(input_file);
	}

	std::array<terraformer::location, 387> base_curve{};
	{
		auto input_file = fopen("testdata/basecurve_2.dat", "rb");
		REQUIRE_NE(input_file, nullptr);
		auto const res = fread(std::data(base_curve), sizeof(terraformer::location), std::size(base_curve), input_file);
		EXPECT_EQ(res, std::size(base_curve));
		fclose(input_file);
	}

	constexpr auto pixel_size = 32.0f;
	auto const res = terraformer::collect_ridge_tree_branch_seeds(
		terraformer::displace_xy(
			base_curve,
			terraformer::displacement_profile{
				.offsets = offsets,
				.sample_period = pixel_size
			}
		)
	);

	{
		auto const normals = res.left.get<1>();
		for(auto normal : normals)
		{	EXPECT_LT(normal[0], 0.0f); }
	}

	{
		auto const normals = res.right.get<1>();
		for(auto normal : normals)
		{	EXPECT_GT(normal[0], 0.0f); }
	}

	EXPECT_EQ(std::size(res.left).get(), 4);
	EXPECT_EQ(std::size(res.right).get(), 3);
}
#endif