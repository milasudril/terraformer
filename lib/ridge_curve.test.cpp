//@	{"target":{"name":"ridge_curve.test"}}

#include "./ridge_curve.hpp"
#include "lib/filters/dft_engine.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_ridge_curve_generate)
{
	terraformer::ridge_curve_description const curve{
		.amplitude = terraformer::horizontal_amplitude{1536.0f},
		.wavelength = terraformer::domain_length{6144.0f},
		.damping = 1.0f/128.0f,
		.flip_direction = false,
		.invert_displacement = false
	};
	terraformer::random_generator rng;
	auto const res = generate(curve, rng, 49152, 1.0f);
	EXPECT_EQ(std::size(res), 49152);

	std::vector<std::complex<double>> input(std::size(res));
	std::transform(std::begin(res), std::end(res), std::begin(input), [](auto val){
		return static_cast<double>(val);
	});
	std::vector<std::complex<double>> output(std::size(res));
	get_plan(std::size(res), terraformer::dft_direction::forward).execute(std::data(input), std::data(output));

	auto max_iter = std::max_element(
		std::begin(output),
		std::begin(output) + std::size(output)/2,
		[](auto a, auto b) {
			return std::abs(a) < std::abs(b);
		}
	);

	EXPECT_EQ(max_iter - std::begin(output), 49152/6144);
}