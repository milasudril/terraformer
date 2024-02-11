#ifndef TERRAFORMER_FPPROPS_HPP
#define TERRAFORMER_FPPROPS_HPP

#include <bit>

namespace terraformer
{
	constexpr bool isnan(float f)
	{
		auto x = std::bit_cast<uint32_t>(f);
		auto pattern = 0x7fc0'0000u;
		return (x&pattern) == pattern ;
	}
}

#endif