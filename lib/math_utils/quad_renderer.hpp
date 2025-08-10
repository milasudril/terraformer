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

	inline auto make_fwd_matrix(quad const& q)
	{
		auto const u = q.p2 - q.p1;
		auto const v = q.p3 - q.p1;
		auto const w = (q.p4 - q.p1) - (u + v);

		return geosimd::mat_4x4{
			u.get(),
			v.get(),
			geosimd::vec_t{0.0f, 0.0f, 1.0f, 0.0f},
			w.get(),
		};
	}

	inline location map_unit_square_to_quad(location loc, quad const& q)
	{
		auto const input_vec = loc - location{};
		auto v_impl = input_vec.get();
		v_impl[3] = v_impl[0]*v_impl[1];

		auto const transform = make_fwd_matrix(q);

		return q.p1 + displacement{transform*v_impl};
	}

	inline auto quad_to_unit_square_compute_delta(quad const& q, displacement current_offset_square, displacement current_offest_quad)
	{
		auto const u = q.p2 - q.p1;
		auto const v = q.p3 - q.p1;
		auto const w = location{} + (q.p4 - q.p1) - (u + v);

		// x[0]' = u[0] x[0] + v[0] x[1] + w[0] x[0] x[1]  <=>  u[0] x[0] + v[0] x[1] + w[0] x[0] x[1] - x[0]' = 0
		// x[1]' = u[1] x[0] + v[1] x[1] + w[1] x[0] x[1]  <=>  u[1] x[0] + v[1] x[1] + w[1] x[0] x[1] - x[1]' = 0

		//
		// J = u[0] + w[0] x[1], v[0] + w[0] x[0]
		//     u[1] + w[1] x[1], v[1] + w[1] x[0]
		//
		auto const x = current_offset_square;
		auto const y = current_offest_quad;

		auto const denom =  (w[0]*v[1] - v[0]*w[1])*x[1]
		                   +(u[0]*w[1] - w[0]*u[1])*x[0]
		                   + u[0]*v[1] - v[0]*u[1];
		return displacement{
			 w[0]*x[0]*y[1] + v[0]*y[1] - y[0]*(x[0]*w[1] + v[1]),
			 x[1]*(y[0]*w[1]     - w[0]*y[1]) - u[0]*y[1] + y[0]*u[1],
			-y[2]
		}/denom;
	}

	inline location map_quad_to_unit_square(quad const& q, location loc)
	{
		auto const u = q.p2 - q.p1;
		auto const v = q.p3 - q.p1;
		auto const w = location{} + (q.p4 - q.p1) - (u + v);

#if 1
		auto const w_factors = shuffle(w.get(), 1, 0, 0, 1);
		auto const uv_factors = shuffle(u.get(), v.get(), 4, 1, 5, 0);
		auto const prod = w_factors*uv_factors;
		auto const sum = shuffle(prod, 0, 1, 2, 3) - shuffle(prod, 2, 3, 2, 3);


		auto const row_0 = shuffle(v.get(), sum, 1, 0, 2, 4)*geosimd::vec_t{1.0f, -1.0f, 1.0f, 1.0f};
		auto const row_1 = shuffle(u.get(), sum, 1, 0, 2, 5)*geosimd::vec_t{-1.0f, 1.0f, 1.0f, 1.0f};
#else
		geosimd::vec_t const row_0{
			v[1],
			-v[0],
			0.0f,
			v[0]*w[1] - v[1]*w[0]
		};

		geosimd::vec_t const row_1{
			-u[1],
			u[0],
			0.0f,
			u[1]*w[0] - u[0]*w[1]
		};
#endif
		geosimd::vec_t const row_2{0.0f, 0.0f, 1.0f, 0.0f};
		geosimd::vec_t const row_3{0.0f, 0.0f, 0.0f, 1.0f};

		std::array const transform{
			row_0,
			row_1,
			row_2,
			row_3
		};
		auto const det = u[0]*v[1] - u[1]*v[0];

		auto const input_vec = (loc - q.p1).get();
		geosimd::vec_t<float, 4> result{};
		for(size_t k = 0; k != 4; ++k)
		{
			result[k] = inner_product(transform[k], input_vec);
		}

		result /= det;
		printf("xy = %.8g\n", result[3]);

		return location{} + displacement{result};
	}

	template<class PixelType, class Shader>
	void render_quad(quad const& q, span_2d<PixelType> output, Shader&& shader)
	{

	}
}

#endif
