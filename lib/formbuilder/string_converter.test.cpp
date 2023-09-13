//@	{"target":{"name":"string_converter.test"}}

#include "./string_converter.hpp"

#include <testfwk/testfwk.hpp>


TESTCASE(terraformer_num_string_converter_to_string)
{
	auto str = terraformer::num_string_converter<terraformer::open_open_interval<int>>::convert(123);
	EXPECT_EQ(str, "123");
}

TESTCASE(terraformer_num_string_converter_from_string_junk_after_data)
{
	terraformer::num_string_converter conv{terraformer::open_open_interval{1, 3}};

	try
	{
		(void)conv.convert("2foo");
		abort();
	}
	catch(terraformer::input_error const& err)
	{
		auto str = std::string_view{err.what()};
		EXPECT_EQ(str, "Expected a number.");
	}
	catch(...)
	{ abort(); }
}

TESTCASE(terraformer_num_string_converter_from_string_good_value)
{
	terraformer::num_string_converter conv{terraformer::open_open_interval{1, 3}};
	auto val = conv.convert("2");
	EXPECT_EQ(val, 2);
}

TESTCASE(terraformer_num_string_converter_from_string_outside_interval)
{
	terraformer::num_string_converter conv{terraformer::open_open_interval{1, 3}};

	try
	{
		(void)conv.convert("3");
		abort();
	}
	catch(terraformer::input_error const& err)
	{
		auto str = std::string_view{err.what()};
		EXPECT_EQ(str, "Input value is out of range. Valid range is ]1, 3[.");
	}
	catch(...)
	{ abort(); }
}

TESTCASE(terraformer_num_string_converter_from_string_long_value)
{
	terraformer::num_string_converter conv{terraformer::open_open_interval{1, 3}};

	try
	{
		(void)conv.convert("3346348906845698406");
		abort();
	}
	catch(terraformer::input_error const& err)
	{
		auto str = std::string_view{err.what()};
		EXPECT_EQ(str, "Input value is out of range. Valid range is ]1, 3[.");
	}
	catch(...)
	{ abort(); }
}

TESTCASE(terraformer_num_string_converter_from_string_junk)
{
	terraformer::num_string_converter conv{terraformer::open_open_interval{1, 3}};

	try
	{
		(void)conv.convert("krdjglökdrjg");
		abort();
	}
	catch(terraformer::input_error const& err)
	{
		auto str = std::string_view{err.what()};
		EXPECT_EQ(str, "Expected a number.");
	}
	catch(...)
	{ abort(); }
}
