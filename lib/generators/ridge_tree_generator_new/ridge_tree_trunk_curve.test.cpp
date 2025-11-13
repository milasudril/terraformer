//@	{"target":{"name":"ridge_tree_trunk_curve.test"}}

#include "./ridge_tree_trunk_curve.hpp"
#include "lib/common/rng.hpp"
#include "lib/common/spaces.hpp"
#include "lib/math_utils/cubic_spline.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ridge_tree_trunk_curve_generate_trunk)
{
#if 0
params.begin.y = (43008.011719, 6144.001465, 0.000000)
params.end.y = (43008.011719, 43008.011719, 0.000000)
params.begin.ddx = (-98304.023438, 0.004297, 0.000000)
params.end.ddx = (98304.023438, 0.004297, 0.000000)
horz_displacement_profile.amplitude = 1222.3099
horz_displacement_profile.damping = 0.70710677
horz_displacement_profile.wavelength = 7680
#endif

	terraformer::random_generator rng{};

	auto const result = generate_trunk(
		terraformer::ridge_tree_trunk_curve{
			.begin = terraformer::cubic_spline_control_point{
				.y = terraformer::location{43008.011719f, 6144.001465f, 0.000000f},
				.ddx = terraformer::displacement{-98304.023438f, 0.004297f, 0.000000f}
			},
			.end = terraformer::cubic_spline_control_point{
				.y = terraformer::location{43008.011719f, 43008.011719f, 0.000000f},
				.ddx = terraformer::displacement{98304.023438f, 0.004297f, 0.000000f}
			}
		},
		terraformer::ridge_tree_branch_displacement_description{
			.amplitude = 1222.3099f,
			.wavelength = 7680.0f,
			.damping = 0.70710677f
		},
		rng
	);

	EXPECT_GE(std::size(result.branches.get<0>().front().get<0>()).get(), 600);
#if 0
	auto const points_out = result.branches.get<0>().front().get<0>();
	auto const indices = points_out.element_indices();
	for(auto k : indices)
	{
		auto const loc = points_out[k];
		printf("%zu %.8g %.8g %.8g\n", k.get(), loc[0], loc[1], loc[2]);
	}
#endif
}