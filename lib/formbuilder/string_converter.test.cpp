//@	{"target":{"name":"string_converter.test"}}

#include "./string_converter.hpp"

#include <testfwk/testfwk.hpp>

#if 0
		using deserialized_type = typename ValidRange::value_type;
		ValidRange range;

		static std::string to_string(deserialized_type value)
		{ return to_string_helper(value); }

		deserialized_type from_string(std::string_view str) const
		{
			deserialized_type val{};
			auto const res = std::from_chars(std::begin(str), std::end(str), val);

			if(res.ptr != std::end(str))
			{ throw input_error{"Expected a number"}; }

			if(res.ec == std::errc{})
			{
				if(within(range, val))
				{ return val; }
				throw input_error{std::string{"Input value is out of range. Valid range is "}.append(to_string(range)).append(".")};
			}

			switch(res.ec)
			{
				case std::errc::result_out_of_range:
				throw input_error{std::string{"Input value is out of range. Valid range is "}.append(to_string(range)).append(".")};

				default:
					throw input_error{"Expected a number"};
			}
		}
#endif


TESTCASE(terraformer_string_converter_to_string)
{
	auto str = terraformer::string_converter<terraformer::open_open_interval<int>>::convert(123);
	EXPECT_EQ(str, "123");
}

TESTCASE(terraformer_string_converter_from_string_junk_after_data)
{
	terraformer::string_converter conv{terraformer::open_open_interval{1, 3}};

	try
	{
		(void)conv.convert("2foo");
		abort();
	}
	catch(terraformer::input_error const& err)
	{
		auto str = std::string_view{err.what()};
		EXPECT_EQ(str, "Expected a number");
	}
	catch(...)
	{ abort(); }
}

TESTCASE(terraformer_string_converter_from_string_good_value)
{
	terraformer::string_converter conv{terraformer::open_open_interval{1, 3}};
	auto val = conv.convert("2");
	EXPECT_EQ(val, 2);
}

TESTCASE(terraformer_string_converter_from_string_outside_interval)
{
	terraformer::string_converter conv{terraformer::open_open_interval{1, 3}};

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

TESTCASE(terraformer_string_converter_from_string_long_value)
{
	terraformer::string_converter conv{terraformer::open_open_interval{1, 3}};

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

TESTCASE(terraformer_string_converter_from_string_junk)
{
	terraformer::string_converter conv{terraformer::open_open_interval{1, 3}};

	try
	{
		(void)conv.convert("krdjglökdrjg");
		abort();
	}
	catch(terraformer::input_error const& err)
	{
		auto str = std::string_view{err.what()};
		EXPECT_EQ(str, "Expected a number");
	}
	catch(...)
	{ abort(); }
}
