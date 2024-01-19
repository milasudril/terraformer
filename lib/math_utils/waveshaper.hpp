#ifndef TERRAFORMER_WAVESHAPER_HPP
#define TERRAFORMER_WAVESHAPER_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/output_range.hpp"
#include <algorithm>
#include <cmath>

namespace terraformer
{
	void sharpen_ridges(span_2d<float> buffer,
		std::ranges::min_max_result<float> input_range,
		float output_amplitude)
	{
		for(uint32_t y = 0; y != buffer.height(); ++y)
		{
			for(uint32_t x = 0; x != buffer.width(); ++x)
			{
				auto const gen_val = buffer(x, y);
				auto const val_normalized = (gen_val - input_range.min)/(input_range.max - input_range.min);
				auto const val = val_normalized != 1.0f?
					1.0f - (1.0f - val_normalized)/std::sqrt(1.0f - val_normalized) :
					1.0f;
				buffer(x, y) = output_amplitude*2.0f*(val - 0.5f);
			}
		}
	}

	void normalize(span_2d<float> buffer,
		std::ranges::min_max_result<float> input_range,
		output_range output_range)
	{
		for(uint32_t y = 0; y != buffer.height(); ++y)
		{
			for(uint32_t x = 0; x != buffer.width(); ++x)
			{
				auto const val_normalized = (buffer(x, y) - input_range.min)/(input_range.max - input_range.min);
				buffer(x, y) = std::lerp(output_range.min, output_range.max, val_normalized);
			}
		}
	}
}

#endif