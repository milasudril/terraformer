//@	{"target":{"name":"utils.test"}}

#include "./utils.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_round_to_n_digits)
{
	{
		auto res = terraformer::round_to_n_digits(512.0f, 1);
		EXPECT_EQ(res, 512.0f);
	}

	{
		auto res = terraformer::round_to_n_digits(900.0f, 1);
		EXPECT_EQ(res, 1024.0f);
	}
}

TESTCASE(teraformer_round_to_odd)
{
	{
		auto res = terraformer::round_to_odd(0.0f);
		EXPECT_EQ(res, 1);
	}

	{
		auto res = terraformer::round_to_odd(0.5f);
		EXPECT_EQ(res, 1);
	}

	{
		auto res = terraformer::round_to_odd(1.5f);
		EXPECT_EQ(res, 1);
	}

	{
		auto res = terraformer::round_to_odd(2.0f);
		EXPECT_EQ(res, 3);
	}

	{
		auto res = terraformer::round_to_odd(2.5f);
		EXPECT_EQ(res, 3);
	}

	{
		auto res = terraformer::round_to_odd(3.0f);
		EXPECT_EQ(res, 3);
	}
}

TESTCASE(terraformer_to_utf32_basic_ascii)
{
	for(size_t k = 0; k != 128; ++k)
	{
		auto input = static_cast<char8_t>(k);
		auto const val = terraformer::to_utf32(std::u8string_view{&input, 1});
		REQUIRE_EQ(std::size(val), 1);
		EXPECT_EQ(val[0], static_cast<char32_t>(k));
	}
}

TESTCASE(terraformer_to_utf32_twobyte_sequences)
{
	constexpr std::u8string_view input{u8"Flygande bäckasiner söka hwila på mjuka tufvor"};
	auto const res = terraformer::to_utf32(input);
	EXPECT_EQ(std::size(res), 46);
	auto const encoded = terraformer::to_utf8(res);
	EXPECT_EQ(encoded, input);
}

TESTCASE(terraformer_to_utf32_incomplete_sequence)
{
	try
	{
		constexpr std::u8string_view input{u8"\xc3"};
		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_to_utf32_invalid_start)
{
	try
	{
		constexpr std::u8string_view input{u8"\x83"};
		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_to_utf32_too_long_sequence)
{
	try
	{
		constexpr std::u8string_view input{u8"\xf8\x1\x82\x83\x84"};
		REQUIRE_EQ(std::size(input), 5);
		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_to_utf32_invalid_second_byte)
{
	try
	{
		constexpr std::u8string_view input{u8"\xc1\x1"};
		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}

	try
	{
		constexpr std::u8string_view input{u8"\xc1\x41"};
		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}

	try
	{
		constexpr std::u8string_view input{u8"\xc1\xc1"};
		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_to_utf32_invalid_codepoint)
{
	try
	{
		std::u8string input;
		constexpr char32_t item{0xd800 + 1};
		input.push_back(static_cast<char8_t>(0xE0 | (item >> 12)));
		input.push_back(static_cast<char8_t>(0x80 | ((item >> 6) & 0x3f)));
		input.push_back(static_cast<char8_t>(0x80 | (item & 0x3f)));

		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}

	try
	{
		constexpr std::u8string_view input{u8"\xc1\x41"};
		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}

	try
	{
		constexpr std::u8string_view input{u8"\xc1\xc1"};
		auto const res = terraformer::to_utf32(input);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_to_utf8_invalid_codepoint)
{
	try
	{
		std::u32string bad_input{0x10ffff + 1};
		auto const res = terraformer::to_utf8(bad_input);
		abort();
	}
	catch(...)
	{}

	try
	{
		std::u32string bad_input{0xd800 + 1};
		auto const res = terraformer::to_utf8(bad_input);
		abort();
	}
	catch(...)
	{}
}

TESTCASE(terraformer_utf8_utf32_roundtrip)
{
	auto input_file = std::unique_ptr<FILE, decltype([](FILE* f){fclose(f);})>(
		fopen(
			"UTF-8-Unicode-Test-Documents/UTF-8_sequence_unseparated/utf8_sequence_0-0x10ffff_assigned_including-unprintable-asis_unseparated.txt",
			"r"
		)
	);
	REQUIRE_NE(input_file, nullptr);

	std::u8string input_string;
	while(true)
	{
		auto ch_in = getc(input_file.get());
		if(ch_in == EOF)
		{ break; }

		input_string.push_back(static_cast<char8_t>(ch_in));
	}
	EXPECT_EQ(std::size(input_string), 880666);

	auto decoded = terraformer::to_utf32(input_string);
	auto encoded = terraformer::to_utf8(decoded);

	EXPECT_EQ(encoded, input_string);
}

TESTCASE(terraformer_scientific_to_natural_faulty_input)
{
	// Too many e
	{
		auto res1 = terraformer::scientific_to_natural("1e655e78");
		EXPECT_EQ(res1, "1e655e78");

		auto res2 = terraformer::scientific_to_natural("1E655e78");
		EXPECT_EQ(res2, "1E655e78");

		auto res3 = terraformer::scientific_to_natural("1E655E78");
		EXPECT_EQ(res3, "1E655E78");

		auto res4 = terraformer::scientific_to_natural("1e655E78");
		EXPECT_EQ(res4, "1e655E78");
	}

	// Start with e
	{
		auto res = terraformer::scientific_to_natural("e54");
		EXPECT_EQ(res, "e54");
	}

	// End with e
	{
		auto res = terraformer::scientific_to_natural("54e");
		EXPECT_EQ(res, "54e");
	}

	// Too many leading chars in mantissa
	{
		auto res = terraformer::scientific_to_natural("+ 347e6");
		EXPECT_EQ(res, "+ 347e6");
	}

	// Wrong sign char in mantissa
	{
		auto res = terraformer::scientific_to_natural("!347e6");
		EXPECT_EQ(res, "!347e6");
	}

	// Too many fraction separators in mantissa
	{
		auto res = terraformer::scientific_to_natural("+34.34.7e6");
		EXPECT_EQ(res, "+34.34.7e6");
	}

	// Fraction separator at front
	{
		auto res = terraformer::scientific_to_natural("+.34e6");
		EXPECT_EQ(res, "+.34e6");
	}

	// Fraction separator at back
	{
		auto res = terraformer::scientific_to_natural("+34.e6");
		EXPECT_EQ(res, "+34.e6");
	}

	// Invalid chars in mantissa (int part)
	{
		auto res = terraformer::scientific_to_natural("+6434a.67e7");
		EXPECT_EQ(res, "+6434a.67e7");
	}

	// Invalid chars in mantissa (frac part)
	{
		auto res = terraformer::scientific_to_natural("+6434.6s7e7");
		EXPECT_EQ(res, "+6434.6s7e7");
	}

	// Too many leading chars in exponent
	{
		auto res = terraformer::scientific_to_natural("+467.67e+ 7");
		EXPECT_EQ(res, "+467.67e+ 7");
	}

	// Invalid chars in exponent
	{
		auto res = terraformer::scientific_to_natural("+467.67e+745d");
		EXPECT_EQ(res, "+467.67e+745d");
	}

	// Wrong sign char in mantissa
	{
		auto res = terraformer::scientific_to_natural("+347e!6");
		EXPECT_EQ(res, "+347e!6");
	}
}

TESTCASE(terraformer_scientific_to_natural_correct_input)
{
	//Positive number no sign, positive exponent no sign
	{
		auto res0 = terraformer::scientific_to_natural("3256.45e0");
		EXPECT_EQ(res0, "3256.45");

		auto res1 = terraformer::scientific_to_natural("3256.45e1");
		EXPECT_EQ(res1, "32564.5");

		auto res2 = terraformer::scientific_to_natural("3256.45e2");
		EXPECT_EQ(res2, "325645");

		auto res3 = terraformer::scientific_to_natural("3256.45e3");
		EXPECT_EQ(res3, "3256450");

		auto res_no_frac = terraformer::scientific_to_natural("325645e3");
		EXPECT_EQ(res_no_frac, "325645000");
	}

	//Positive number no sign, positive exponent sign
	{
		auto res0 = terraformer::scientific_to_natural("3256.45e+0");
		EXPECT_EQ(res0, "3256.45");

		auto res1 = terraformer::scientific_to_natural("3256.45e+1");
		EXPECT_EQ(res1, "32564.5");

		auto res2 = terraformer::scientific_to_natural("3256.45e+2");
		EXPECT_EQ(res2, "325645");

		auto res3 = terraformer::scientific_to_natural("3256.45e+3");
		EXPECT_EQ(res3, "3256450");

		auto res_no_frac = terraformer::scientific_to_natural("325645e+03");
		EXPECT_EQ(res_no_frac, "325645000");
	}

	//Positive number sign, positive exponent sign
	{
		auto res0 = terraformer::scientific_to_natural("+3256.45e+0");
		EXPECT_EQ(res0, "3256.45");

		auto res1 = terraformer::scientific_to_natural("+3256.45e+1");
		EXPECT_EQ(res1, "32564.5");

		auto res2 = terraformer::scientific_to_natural("+3256.45e+2");
		EXPECT_EQ(res2, "325645");

		auto res3 = terraformer::scientific_to_natural("+3256.45e+3");
		EXPECT_EQ(res3, "3256450");

		auto res_no_frac = terraformer::scientific_to_natural("+325645e+09");
		EXPECT_EQ(res_no_frac, "325645000000000");
	}

	//Negative number positive exponent
	{
		auto res0 = terraformer::scientific_to_natural("-3256.45e0");
		EXPECT_EQ(res0, "-3256.45");

		auto res1 = terraformer::scientific_to_natural("-3256.45e1");
		EXPECT_EQ(res1, "-32564.5");

		auto res2 = terraformer::scientific_to_natural("-3256.45e2");
		EXPECT_EQ(res2, "-325645");

		auto res3 = terraformer::scientific_to_natural("-3256.45e3");
		EXPECT_EQ(res3, "-3256450");

		auto res_no_frac = terraformer::scientific_to_natural("-325645e3");
		EXPECT_EQ(res_no_frac, "-325645000");
	}

	//Negative number negative exponent
	{
		auto res0 = terraformer::scientific_to_natural("-3256.45e-0");
		EXPECT_EQ(res0, "-3256.45");

		auto res1 = terraformer::scientific_to_natural("-3256.45e-1");
		EXPECT_EQ(res1, "-325.645");

		auto res2 = terraformer::scientific_to_natural("-3256.45e-2");
		EXPECT_EQ(res2, "-32.5645");

		auto res4 = terraformer::scientific_to_natural("-3256.45e-4");
		EXPECT_EQ(res4, "-0.325645");

		auto res5 = terraformer::scientific_to_natural("-3256.45e-5");
		EXPECT_EQ(res5, "-0.0325645");
	}
}

TESTCASE(terraformer_make_unique_handle)
{
	auto x = std::make_unique<int>(124);
	auto ptr = x.get();
	auto handle = terraformer::make_unique_handle(std::move(x));
	EXPECT_EQ(handle.get(), ptr);
	EXPECT_EQ(x.get(), nullptr);
}