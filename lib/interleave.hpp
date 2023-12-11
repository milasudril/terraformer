#ifndef TERRAFORMER_INTERLEAVE_HPP
#define TERRAFORMER_INTERLEAVE_HPP

#include <vector>
#include <span>

namespace terraformer
{
	template<class T>
	std::vector<T> interleave(std::span<T const> input)
	{
		std::vector<T> ret;
		ret.reserve(std::size(input));
		for(size_t k = 0; k != std::size(input)/2; ++k)
		{ ret.push_back(input[2*k]); }

		if(std::size(input) % 2 != 0)
		{ ret.push_back(input.back()); }

		for(size_t k = 0; k != std::size(input)/2; ++k)
		{ ret.push_back(input[2*k + 1]); }

		return ret;
	}
}

#endif