//@	{"target":{"name":"ridge_tree_branch.test"}}

#include "./ridge_tree_branch.hpp"

#include "./rng.hpp"
#include <random>

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ridge_tree_branch_plus_minus_plus)
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
	terraformer::ridge_tree_branch const branch{
		base_curve,
		terraformer::displacement_profile{
			.offsets = offsets,
			.sample_period = 1.15f*dx
		}
	};

	{
		auto const left_seeds = branch.left_seeds();
		EXPECT_EQ(std::size(left_seeds.branch_points), 2);
		EXPECT_EQ(std::size(left_seeds.delimiter_points), 1);
		EXPECT_EQ(left_seeds.first_is_branch, true);
	}

	{
		auto const right_seeds = branch.right_seeds();
		EXPECT_EQ(std::size(right_seeds.branch_points), 1);
		EXPECT_EQ(std::size(right_seeds.delimiter_points), 2);
		EXPECT_EQ(right_seeds.first_is_branch, false);
	}
}

TESTCASE(terraformer_ridge_tree_branch_minus_plus_minus)
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
	terraformer::ridge_tree_branch const branch{
		base_curve,
		terraformer::displacement_profile{
			.offsets = offsets,
			.sample_period = 1.15f*dx
		}
	};

	{
		auto const left_seeds = branch.left_seeds();
		EXPECT_EQ(std::size(left_seeds.branch_points), 1);
		EXPECT_EQ(std::size(left_seeds.delimiter_points), 2);
		EXPECT_EQ(left_seeds.first_is_branch, false);
	}

	{
		auto const right_seeds = branch.right_seeds();
		EXPECT_EQ(std::size(right_seeds.branch_points), 2);
		EXPECT_EQ(std::size(right_seeds.delimiter_points), 1);
		EXPECT_EQ(right_seeds.first_is_branch, true);
	}
}

TESTCASE(terraformer_ridge_tree_branch_plus_minus_plus_minus)
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
	terraformer::ridge_tree_branch const branch{
		base_curve,
		terraformer::displacement_profile{
			.offsets = offsets,
			.sample_period = 1.15f*dx
		}
	};

	{
		auto const left_seeds = branch.left_seeds();
		EXPECT_EQ(std::size(left_seeds.branch_points), 2);
		EXPECT_LT(
			distance(left_seeds.branch_points.get<0>()[0], (terraformer::location{0.17f, -0.015f, 0.0f})),
			0.05f
		);
		EXPECT_LT(left_seeds.branch_points.get<1>()[0][1], 0.0f);

		EXPECT_EQ(std::size(left_seeds.delimiter_points), 2);
		EXPECT_LT(
			distance(left_seeds.delimiter_points.get<0>()[0], (terraformer::location{0.3f, 0.35f, 0.0f})),
			0.05f
		);
		EXPECT_LT(left_seeds.delimiter_points.get<1>()[0][1], 0.0f);

		EXPECT_EQ(left_seeds.first_is_branch, true);
	}

	{
		auto const right_seeds = branch.right_seeds();
		EXPECT_EQ(std::size(right_seeds.branch_points), 2);
		EXPECT_LT(
			distance(right_seeds.branch_points.get<0>()[0], (terraformer::location{1.0f, 0.18f, 0.0f})),
			0.05f
		);
		EXPECT_GT(right_seeds.branch_points.get<1>()[0][1], 0.0f);


		EXPECT_EQ(std::size(right_seeds.delimiter_points), 2);
		EXPECT_LT(
			distance(right_seeds.delimiter_points.get<0>()[0], (terraformer::location{0.62f, 0.1f, 0.0f})),
			0.05f
		);
		EXPECT_GT(right_seeds.delimiter_points.get<1>()[0][1], 0.0f);

		EXPECT_EQ(right_seeds.first_is_branch, true);
	}
}

TESTCASE(terraformer_ridge_tree_branch_minus_plus_minus_plus)
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
	terraformer::ridge_tree_branch const branch{
		base_curve,
		terraformer::displacement_profile{
			.offsets = offsets,
			.sample_period = 1.15f*dx
		}
	};

	{
		auto const left_seeds = branch.left_seeds();
		EXPECT_EQ(std::size(left_seeds.branch_points), 2);
		EXPECT_EQ(std::size(left_seeds.delimiter_points), 2);
		EXPECT_EQ(left_seeds.first_is_branch, false);
	}

	{
		auto const right_seeds = branch.right_seeds();
		EXPECT_EQ(std::size(right_seeds.branch_points), 2);
		EXPECT_EQ(std::size(right_seeds.delimiter_points), 2);
		EXPECT_EQ(right_seeds.first_is_branch, false);
	}
}

TESTCASE(terraformer_ridge_tree_branch_random_data)
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
		base_curve[k] += d*terraformer::displacement{U(rng), U(rng), U(rng)};
	}

	constexpr auto pixel_size = 32.0f;
	terraformer::ridge_tree_branch const branch{
		base_curve,
		terraformer::displacement_profile{
			.offsets = offsets,
			.sample_period = pixel_size,
		}
	};

	printf("%zu %zu\n", std::size(branch.left_seeds().branch_points), std::size(branch.right_seeds().branch_points));
}