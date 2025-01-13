//@	{"target":{"name":"utils.o"}}

#include "./utils.hpp"
#include <bit>
#include <algorithm>

namespace
{
	char to_hex_digit(int value)
	{ return static_cast<char>(value < 10 ? '0' + value : 'A' + (value - 10)); }

	bool is_hex_digit(char digit)
	{
		return (digit >= '0' && digit <= '9') || (digit >= 'A' && digit <= 'F');
	}

	int from_hex_digit(char digit)
	{
		return digit < '0' + 10 ? digit - '0' : 10 + digit  - 'A';
	}
}

void terraformer::bytes_to_hex(char* dest,  void const* src, size_t dest_length)
{
	auto ptr = reinterpret_cast<uint8_t const*>(src);
	auto const src_end = ptr + dest_length / 2;
	while(ptr != src_end)
	{
		auto const val = *ptr;
		auto const msb = (val & 0xf0) >> 4;
		auto const lsb = (val & 0x0f);
		*dest = to_hex_digit(msb);
		*(dest + 1) = to_hex_digit(lsb);
		dest += 2;
		++ptr;
	}
}

terraformer::hex_to_bytes_result terraformer::hex_to_bytes(void* dest, char const* src, size_t dest_length)
{
	auto ptr = src;
	auto const src_end = ptr + 2*dest_length;
	auto dest_ptr = reinterpret_cast<uint8_t*>(dest);
	while(ptr != src_end)
	{
		auto const ch_msb = *(ptr);
		auto const ch_lsb = *(ptr + 1);
		if(!is_hex_digit(ch_msb)) [[unlikely]]
		{ return hex_to_bytes_result{.ptr = ptr}; }
		if(!is_hex_digit(ch_lsb)) [[unlikely]]
		{ return hex_to_bytes_result{.ptr = ptr + 1}; }

		auto const val = (from_hex_digit(ch_msb) << 4) | from_hex_digit(ch_lsb);
		*dest_ptr = static_cast<uint8_t>(val);
		++dest_ptr;
		ptr += 2;
	}
	return hex_to_bytes_result{.ptr = src_end};
}

std::u8string terraformer::to_utf8(std::u32string_view str)
{
	std::u8string ret;
	for(auto item : str)
	{
		if(!is_valid(item))
		{ throw std::runtime_error{"Codepoint out of range"}; }

		if(item <= 0x7f)
		{ ret.push_back(static_cast<char8_t>(item)); }
		else
		if(item <= 0x7ff)
		{
			ret.push_back(static_cast<char8_t>(0xC0 | (item >> 6)));
			ret.push_back(static_cast<char8_t>(0x80 | (item & 0x3f)));
		}
		else
		if(item <= 0xffff)
		{
			ret.push_back(static_cast<char8_t>(0xE0 | (item >> 12)));
			ret.push_back(static_cast<char8_t>(0x80 | ((item >> 6) & 0x3f)));
			ret.push_back(static_cast<char8_t>(0x80 | (item & 0x3f)));
		}
		else
		if(item <= 0x10ffff)
		{
			ret.push_back(static_cast<char8_t>(0xF0 | (item >> 18)));
			ret.push_back(static_cast<char8_t>(0x80 | ((item >> 12) & 0x3f)));
			ret.push_back(static_cast<char8_t>(0x80 | ((item >> 6) & 0x3f)));
			ret.push_back(static_cast<char8_t>(0x80 | (item & 0x3f)));
		}
	}
	return ret;
}

std::u32string terraformer::to_utf32(std::u8string_view str)
{
	std::u32string ret;

	struct decoder_state
	{
		decoder_state() = default;

		explicit decoder_state(char32_t item):
			code_units_to_read{std::max(std::countl_one(static_cast<uint8_t>(item)), 1)},
			code_units_read{0},
			current_codepoint{0},
			current_shift{6*(code_units_to_read - 1)}
		{
			if(code_units_to_read > 4 || (code_units_to_read == 1 && (item & 0x80)))
			{ throw std::runtime_error{"Invalid UTF-8 sequence"}; }
		}

		bool at_end() const
		{ return code_units_to_read == code_units_read; }

		int code_units_to_read{0};
		int code_units_read{0};
		char32_t current_codepoint{0};
		int current_shift{0};
	};

	decoder_state current_state{};
	for(auto item : str)
	{
		if(current_state.at_end())
		{
			current_state = decoder_state{item};
			static constexpr std::array<uint8_t, 4> init_mask{
				0b0111'1111,
				0b0001'1111,
				0b0000'1111,
				0b0000'0111
			};
			item &= init_mask[current_state.code_units_to_read - 1];
		}
		else
		{
			if((item&0xc0) != 0x80)
			{ throw std::runtime_error{"Invalid UTF-8 sequence"}; }
			item &= 0x3f;
		}
		current_state.current_codepoint |= (item << current_state.current_shift);
		++current_state.code_units_read;
		current_state.current_shift -= 6;

		if(current_state.at_end())
		{
			if(!is_valid(current_state.current_codepoint))
			{ throw std::runtime_error{"Invalid UTF-8 sequence"}; }
			ret.push_back(current_state.current_codepoint);
		}
	}

	if(!current_state.at_end())
	{ throw std::runtime_error{"Invalid UTF-8 sequence"}; }

	return ret;
}

std::string terraformer::scientific_to_natural(std::string_view input)
{
	auto const is_e = [](char value){ return value == 'E' || value == 'e'; };
	auto const is_digit = [](char value){ return value >= '0' && value <= '9'; };
	auto const is_frac_separator = [](char value){ return value == '.'; };
	auto const valid_char_mantissa = [is_digit, is_frac_separator](char value){
		return is_digit(value) || is_frac_separator(value);
	};

	if(std::ranges::count_if(input, is_e) != 1)
	{ return std::string{input}; }

	auto const e_offset = std::ranges::find_if(input, is_e);
	if(e_offset == &input.back() || e_offset == &input.front())
	{ return std::string{input}; }

	// Since delimiter is not at begin or end, mantissa must have length 1
	auto const mantissa = std::string_view{std::begin(input), e_offset};
	auto const mantissa_digits_begin = std::ranges::find_if(mantissa, is_digit);
	if(mantissa_digits_begin - std::begin(mantissa) >= 2)
	{ return std::string{input}; }
	auto const mantissa_digits = std::string_view{mantissa_digits_begin, std::end(mantissa)};
	if(std::ranges::count_if(mantissa_digits, is_frac_separator) > 1)
	{ return std::string{input}; }
	auto const frac_separator_loc = std::ranges::find_if(mantissa_digits, is_frac_separator);
	if(frac_separator_loc == &mantissa_digits.front() || frac_separator_loc == &mantissa_digits.back())
	{ return std::string{input}; }
	if(!std::ranges::all_of(mantissa_digits, valid_char_mantissa))
	{ return std::string{input}; }
	auto const mantissa_sign = (mantissa_digits_begin != std::begin(mantissa))?
		mantissa.front() : '+';
	if(mantissa_sign != '+' && mantissa_sign != '-')
	{ return std::string{input}; }
	//auto const mantissa_digit_count = std::ranges::count_if(mantissa_digits, is_digit);


	// Since delimiter is not at begin or end, exponent must have length 1
	auto const exponent = std::string_view{e_offset + 1, std::end(input)};
	auto const exponent_digits_begin = std::ranges::find_if(exponent, is_digit);
	if(exponent_digits_begin - std::begin(exponent) >= 2)
	{ return std::string{input}; }
	auto const exponent_digits = std::string_view{exponent_digits_begin, std::end(exponent)};
	if(!std::ranges::all_of(exponent_digits, is_digit))
	{ return std::string{input}; }
	auto const exponent_sign = (exponent_digits_begin != std::begin(exponent))?
		exponent.front() : '+';
	if(exponent_sign != '+' && exponent_sign != '-')
	{ return std::string{input}; }

	std::string ret;
	if(mantissa_sign != '+')
	{ ret += mantissa_sign; }

	auto exp_value = 0;
	std::from_chars(std::begin(exponent_digits), std::end(exponent_digits), exp_value);

	if(exponent_sign == '+')
	{

	}

	return ret;
}