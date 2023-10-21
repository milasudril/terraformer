//@	{"target":{"name":"calculator.test"}}

#include "./calculator.hpp"

#include <numbers>

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_calculator_single_number)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("5.25"), 5.25f);
}

TESTCASE(terraformer_calculator_junk_after_number)
{
	terraformer::calculator calc;
	try
	{
		calc.convert("5.25abc");
		abort();
	}
	catch(terraformer::input_error const& err)
	{
		EXPECT_EQ(err.what(), std::string_view{"Unknown constant `5.25abc`"});
	}
}

TESTCASE(terraformer_calculator_unknown_constant)
{
	terraformer::calculator calc;
	try
	{
		calc.convert("bajs");
		abort();
	}
	catch(terraformer::input_error const& err)
	{
		EXPECT_EQ(err.what(), std::string_view{"Unknown constant `bajs`"});
	}
}

TESTCASE(terraformer_calculator_singe_constant)
{
	terraformer::calculator calc;
	auto const res = calc.convert("phi");
	EXPECT_EQ(res, std::numbers::phi_v<float>);
}