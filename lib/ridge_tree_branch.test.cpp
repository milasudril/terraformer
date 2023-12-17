//@	{"target":{"name":"ridge_tree_branch.test"}}

#include "./ridge_tree_branch.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ridge_tree_plus_minus_plus)
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