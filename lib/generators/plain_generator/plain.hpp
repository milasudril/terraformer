
#include "lib/common/span_2d.hpp"

namespace terraformer
{
	struct plain_corner_descriptor
	{
		float elevation = 840.0f;
		float ddx = 0.0f;
		float ddy = 0.0f;
	};

	struct plain_descriptor
	{
		plain_corner_descriptor nw;
		plain_corner_descriptor ne;
		plain_corner_descriptor sw;
		plain_corner_descriptor se;
	};
}