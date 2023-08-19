#ifndef TERRAFORMER_POLYLINE_HPP
#define TERRAFORMER_POLYLINE_HPP

#include "lib/common/spaces.hpp"

namespace terraformer
{
	struct uniform_polyline_params
	{
		location start_location;
		size_t point_count;
		float dx;
	};
}
#endif