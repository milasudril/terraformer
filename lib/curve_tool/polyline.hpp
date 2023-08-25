#ifndef TERRAFORMER_POLYLINE_HPP
#define TERRAFORMER_POLYLINE_HPP

#include "lib/common/spaces.hpp"

namespace terraformer
{
	struct polyline_location_params
	{
		size_t point_count;
		float dx;
		location start_location;
	};

	struct polyline_displacement_params
	{
		size_t point_count;
		float dx;
	};
}
#endif