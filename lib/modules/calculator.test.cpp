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

TESTCASE(terraformer_calculator_single_constant)
{
	terraformer::calculator calc;
	auto const res = calc.convert("phi");
	EXPECT_EQ(res, std::numbers::phi_v<float>);
}

TESTCASE(terraformer_calculator_unknown_command)
{
	terraformer::calculator calc;
	try
	{
		(void)calc.convert("blah()");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"Unknown function `blah`"}); }
}

TESTCASE(terraformer_calculator_add)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("add(1, 2, 3, 4)"), 10.0f);
	EXPECT_EQ(calc.convert("add()"), 0.0f);
}

TESTCASE(terraformer_calculator_mul)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("mul(1, 2, 3, 4)"), 24.0f);
	EXPECT_EQ(calc.convert("mul()"), 1.0f);
}

TESTCASE(terraformer_calculator_addinv)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("addinv(1)"), -1.0f);
	EXPECT_EQ(calc.convert("addinv(-1)"), 1.0f);
	EXPECT_EQ(calc.convert("addinv(3)"), -3.0f);
	try
	{
		(void)calc.convert("addinv()");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`addinv` requires exactly one argument"}); }
}

TESTCASE(terraformer_calculator_mulinv)
{
	terraformer::calculator calc;
 	EXPECT_EQ(calc.convert("mulinv(4)"), 1.0f/4.0f);
	EXPECT_EQ(calc.convert("mulinv(2)"), 1.0f/2.0f);
	try
	{
		(void)calc.convert("mulinv()");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`mulinv` requires exactly one argument"}); }
}

TESTCASE(terraformer_calculator_sub)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("sub(4, 1)"), 3.0f);
	try
	{
		(void)calc.convert("sub(1, 2, 3)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`sub` requires exactly two arguments"}); }
}

TESTCASE(terraformer_calculator_div)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("div(12,3)"), 4.0f);
	EXPECT_EQ(calc.convert("div(9,3)"), 3.0f);
	try
	{
		(void)calc.convert("div(1, 2, 3)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`div` requires exactly two arguments"}); }
}

TESTCASE(terraformer_calculator_complement)
{
	terraformer::calculator calc;
 	EXPECT_EQ(calc.convert("compl(0.75)"), 1.0f/4.0f);
	EXPECT_EQ(calc.convert("compl(0.25)"), 3.0f/4.0f);
	try
	{
		(void)calc.convert("compl(3,6,7,8)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`compl` requires exactly one argument"}); }
}

TESTCASE(terraformer_calculator_square)
{
	terraformer::calculator calc;
 	EXPECT_EQ(calc.convert("square(2)"), 4.0f);
	EXPECT_EQ(calc.convert("square(3)"), 9.0f);
	try
	{
		(void)calc.convert("square(3,6,7,8)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`square` requires exactly one argument"}); }
}

TESTCASE(terraformer_calculator_sroot)
{
	terraformer::calculator calc;
 	EXPECT_EQ(calc.convert("sroot(4)"), 2.0f);
	EXPECT_EQ(calc.convert("sroot(9)"), 3.0f);
	try
	{
		(void)calc.convert("sroot(3,6,7,8)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`sroot` requires exactly one argument"}); }
}

TESTCASE(terraformer_calculator_qroot)
{
	terraformer::calculator calc;
 	EXPECT_EQ(calc.convert("qroot(27)"), 3.0f);
	EXPECT_EQ(calc.convert("qroot(64)"), 4.0f);
	try
	{
		(void)calc.convert("qroot(3,6,7,8)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`qroot` requires exactly one argument"}); }
}

TESTCASE(terraformer_calculator_nth_root)
{
	terraformer::calculator calc;
 	EXPECT_EQ(calc.convert("nth_root(3, 64)"), 4.0f);
	EXPECT_EQ(calc.convert("nth_root(2, 64)"), 8.0f);
	try
	{
		(void)calc.convert("nth_root(3)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`nth_root` requires exactly two arguments"}); }
}

TESTCASE(terraformer_calculator_exp)
{
	terraformer::calculator calc;
 	EXPECT_EQ(calc.convert("exp(3)"), 8.0f);
	EXPECT_EQ(calc.convert("exp(3, 2)"), 9.0f);
	try
	{
		(void)calc.convert("exp(3, 4, 5)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`exp` requires one or two arguments"}); }
}

TESTCASE(terraformer_calculator_log)
{
	terraformer::calculator calc;
 	EXPECT_EQ(calc.convert("log(8)"), 3.0f);
	EXPECT_EQ(calc.convert("log(3, 9)"), 2.0f);
	try
	{
		(void)calc.convert("log(3, 4, 5)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`log` requires one or two arguments"}); }
}

TESTCASE(terraformer_calculator_count)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("count(1, 2, 3, 4)"), 4.0f);
}

TESTCASE(terraformer_calculator_aritmean)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("aritmean(0, 2, 2, 3)"), 7.0f/4.0f);

	try
	{
		(void)calc.convert("aritmean()");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`aritmean` requires at least one argument"}); }
}

TESTCASE(terraformer_calculator_geommean)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("geommean(1, 2, 2, 3)"), std::pow(12.0f, 1.0f/4.0f));

	try
	{
		(void)calc.convert("geommean()");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`geommean` requires at least one argument"}); }
}

TESTCASE(terraformer_calculator_median)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("median(0, 2, 3, 3)"), 2.5f);
	EXPECT_EQ(calc.convert("median(11, 2, 2, 3, 10)"), 3.0f);
	try
	{
		(void)calc.convert("median()");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`median` requires at least one argument"}); }
}

TESTCASE(terraformer_calculator_norm)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("norm(0, 2, 2, 3)"), std::sqrt(17.0f));
}

TESTCASE(terraformer_calculator_rms)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("rms(0, 2, 2, 3, 0)"), std::sqrt(17.0f/5.0f));

	try
	{
		(void)calc.convert("rms()");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`rms` requires at least one argument"}); }
}

TESTCASE(terraformer_calculator_fibseq)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("fibseq(3)"), 2.0f);
	EXPECT_EQ(calc.convert("fibseq(4)"), 3.0f);
	EXPECT_EQ(calc.convert("fibseq(5)"), 5.0f);
	EXPECT_EQ(calc.convert("fibseq(6)"), 8.0f);

	try
	{
		(void)calc.convert("fibseq(3, 4, 5)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`fibseq` requires exactly one argument"}); }
}

TESTCASE(terraformer_calculator_mersenneseq)
{
	terraformer::calculator calc;
	EXPECT_EQ(calc.convert("mersenneseq(3)"), 7.0f);
	EXPECT_EQ(calc.convert("mersenneseq(4)"), 15.0f);
	EXPECT_EQ(calc.convert("mersenneseq(5)"), 31.0f);
	EXPECT_EQ(calc.convert("mersenneseq(6)"), 63.0f);

	try
	{
		(void)calc.convert("mersenneseq(3, 4, 5)");
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"`mersenneseq` requires exactly one argument"}); }
}