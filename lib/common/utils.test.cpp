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