//@	{"target":{"name":"ridge_curve.test"}}

#include "./ridge_curve.hpp"
#include "lib/math_utils/dft_engine.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ridge_curve_generate)
{
	terraformer::ridge_tree_branch_displacement_description const curve{
		.amplitude = 1536.0f,
		.wavelength = 6144.0f,
		.damping = 1.0f/std::sqrt(2.0f)
	};
	terraformer::random_generator rng;
	auto const res = generate(curve, rng, terraformer::array_size<float>{49152}, 1.0f);
	EXPECT_EQ(std::size(res).get(), 49152);

	terraformer::single_array input{terraformer::array_size<std::complex<float>>{std::size(res)}};
	std::transform(std::begin(res), std::end(res), std::begin(input), [](auto val){
		return static_cast<float>(val);
	});
	terraformer::single_array output(std::size(input));
	get_plan(std::size(res).get(), terraformer::dft_direction::forward).execute(std::data(input), std::data(output));

	auto max_iter = std::max_element(
		std::begin(output),
		std::begin(output) + std::size(output).get()/2,
		[](auto a, auto b) {
			return std::abs(a) < std::abs(b);
		}
	);

	EXPECT_EQ(max_iter - std::begin(output), 49152/6144);
}