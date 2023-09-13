//@	{"target":{"name":"utils.o"}}

#include "./utils.hpp"

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