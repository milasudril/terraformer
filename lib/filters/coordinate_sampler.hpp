#ifndef TERRAFORMER_LIB_IMAGE_SAMPLER_HPP
#define TERRAFORMER_LIB_IMAGE_SAMPLER_HPP

#include "lib/common/span_2d.hpp"

namespace terraformer
{
	template<class Pred>
	concept pixel_selection_predicate = requires(Pred pred, uint32_t x, uint32_t y)
	{
		{pred(x, y)} -> std::same_as<bool>;
	};

	template<by_value_selection_predicate Pred>
	auto sample(uint32_t width, uint32_t height, Pred&& pred)
	{
		std::vector<pixel_coordinates> ret;
		for(uint32_t y = 0; y != height; ++y)
		{
			for(uint32_t x = 0; x != width; ++x)
			{
				if(pred(values(x, y)))
				{ ret.push_back(pixel_coordinates{x, y}); }
			}
		}
		return ret;
	}
}

#endif