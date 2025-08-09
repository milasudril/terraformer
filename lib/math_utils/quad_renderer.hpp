#ifndef TERRAFORMER_QUAD_RENDERER_HPP
#define TERRAFORMER_QUAD_RENDERER_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/spaces.hpp"
#include <array>

namespace terraformer
{
	struct quad
	{
		location p1;
		location p2;
		location p3;
		location p4;
	};

	inline location map_unit_square_to_quad(location loc, quad const& q)
	{
		loc[2] = loc[0]*loc[1];
		auto const input_vec = loc - location{};

		auto const u = q.p2 - q.p1;
		auto const v = q.p3 - q.p1;
		auto const w = (q.p4 - q.p1) - (u + v);

		geosimd::mat_4x4 const transform{
			u.get(),
			v.get(),
			w.get(),
			geosimd::vec_t{0.0f, 0.0f, 0.0f, 1.0f}
		};

		return q.p1 + input_vec.apply(transform);
	}

	template<class PixelType, class Shader>
	void render_quad(quad const& q, span_2d<PixelType> output, Shader&& shader)
	{

	}
}

#endif
