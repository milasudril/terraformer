//@	{"target":{"name":"peaks.test"}}

#include "./peaks.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_for_each_peak_1d)
{
	constexpr size_t num_periods = 8;
	constexpr size_t steps_per_preiod = 16;
	std::array<float, steps_per_preiod*num_periods> vals;
	for(size_t k = 0; k != std::size(vals); ++k)
	{
		auto const x = static_cast<float>(k)/static_cast<float>(steps_per_preiod);
		vals[k] = std::sin(2.0f*std::numbers::pi_v<float>*x);
	}

	size_t num_peaks = 0;
	terraformer::for_each_peak(vals, [&num_peaks](size_t index, float value){
		EXPECT_EQ((index - steps_per_preiod/4)/steps_per_preiod, num_peaks);
		EXPECT_EQ(value, 1.0f);
		++num_peaks;
	});

	EXPECT_EQ(num_peaks, num_periods);
}