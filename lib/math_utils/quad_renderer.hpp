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
		auto const input_vec = loc - location{};
		auto v_impl = input_vec.get();
		v_impl[3] = v_impl[0]*v_impl[1];


		auto const u = q.p2 - q.p1;
		auto const v = q.p3 - q.p1;
		auto const w = (q.p4 - q.p1) - (u + v);

		geosimd::mat_4x4 const transform{
			u.get(),
			v.get(),
			geosimd::vec_t{0.0f, 0.0f, 1.0f, 0.0f},
			w.get(),
		};

		return q.p1 + displacement{transform*v_impl};
	}

	template<class PixelType, class Shader>
	void render_quad(quad const& q, span_2d<PixelType> output, Shader&& shader)
	{

	}
}

#endif
