#ifndef TERRAFORMER_QUAD_RENDERER_HPP
#define TERRAFORMER_QUAD_RENDERER_HPP

#include "lib/common/span_2d.hpp"
#include "lib/common/spaces.hpp"

#include <array>
#include <geosimd/line.hpp>

namespace terraformer
{
	struct quad
	{
		location origin;
		location lower_right;
		location upper_left;
		location remote;
	};

	inline bool has_crossed_edges(quad const& q)
	{
		{
			geosimd::line l1{q.origin, q.lower_right};
			geosimd::line l2{q.upper_left, q.remote};
			auto const param = intersect_2d(l1, l2);
			if(param.has_value())
			{
				if((param->a.get() >= 0.0f && param->a.get() <= 1.0f)
					&& (param->b.get() >= 0.0f && param->b.get() <= 1.0f))
				{ return true; }
			}
		}

		{
			geosimd::line l1{q.origin, q.upper_left};
			geosimd::line l2{q.lower_right, q.remote};
			auto const param = intersect_2d(l1, l2);
			if(param.has_value())
			{
				if((param->a.get() >= 0.0f && param->a.get() <= 1.0f)
					&& (param->b.get() >= 0.0f && param->b.get() <= 1.0f))
				{ return true; }
			}
		}

		return false;
	}

	struct quad_aabb
	{
		location min;
		location max;
	};

	inline quad_aabb make_aabb(quad const& q)
	{
		auto const v1 = (q.origin - location{}).get();
		auto const v2 = (q.lower_right - location{}).get();
		auto const v3 = (q.upper_left - location{}).get();
		auto const v4 = (q.remote - location{}).get();

		return quad_aabb{
			.min = location{} + displacement{min(v1, min(v2, min(v3, v4)))},
			.max = location{} + displacement{max(v1, max(v2, max(v3, v4)))}
		};
	}

	struct quad_params
	{
		location origin;
		displacement u;
		displacement v;
		displacement w;
	};

	inline auto make_quad_params(quad const& q)
	{
		auto const u = q.lower_right - q.origin;
		auto const v = q.upper_left - q.origin;
		return quad_params{
			.origin = q.origin,
			.u = u,
			.v = v,
			.w = (q.remote - q.origin) - (u + v)
		};
	}

	struct quad_fwd_matrix
	{
		location origin;
		geosimd::mat_4x4<float> mat;
	};

	inline auto make_fwd_matrix(quad_params const& q)
	{
		return quad_fwd_matrix{
			.origin = q.origin,
			.mat = geosimd::mat_4x4{
				q.u.get(),
				q.v.get(),
				geosimd::vec_t{0.0f, 0.0f, 1.0f, 0.0f},
				q.w.get(),
			}
		};
	}

	inline displacement map_unit_square_to_quad_rel(displacement input_vec, quad_fwd_matrix const& m)
	{
		auto v_impl = input_vec.get();
		v_impl[3] = v_impl[0]*v_impl[1];

		return displacement{m.mat*v_impl};
	}

	inline location map_unit_square_to_quad(location loc, quad_fwd_matrix const& m)
	{ return m.origin + map_unit_square_to_quad_rel(loc - location{}, m); }

	inline location map_unit_square_to_quad(location loc, quad const& q)
	{ return q.origin + map_unit_square_to_quad_rel(loc - location{}, make_fwd_matrix(make_quad_params(q))); }

	inline auto quad_to_unit_square_compute_delta(quad_params const& q, displacement current_offset_square, displacement current_offest_quad)
	{
		auto const u = q.u;
		auto const v = q.v;
		auto const w = q.w;

		// y[0] = u[0] x[0] + v[0] x[1] + w[0] x[0] x[1]  <=>  u[0] x[0] + v[0] x[1] + w[0] x[0] x[1] - y[0] = 0
		// y[1] = u[1] x[0] + v[1] x[1] + w[1] x[0] x[1]  <=>  u[1] x[0] + v[1] x[1] + w[1] x[0] x[1] - y[1] = 0

		//
		// J = u[0] + w[0] x[1], v[0] + w[0] x[0]
		//     u[1] + w[1] x[1], v[1] + w[1] x[0]
		//
		auto const x = current_offset_square;
		auto const y = -current_offest_quad;
		auto const denom = (w[0]*v[1] - v[0]*w[1])*x[1] + x[0]*(u[0]*w[1] - w[0]*u[1]) + u[0]*v[1] - u[1]*v[0];

		return displacement{
			 x[0]*(w[0]*y[1] - w[1]*y[0]) + v[0]*y[1] - y[0]*v[1],
			 x[1]*(w[1]*y[0] - w[0]*y[1]) - u[0]*y[1] + y[0]*u[1],
			-y[2]
		}/denom;
	}

	inline auto quad_to_unit_square_compute_initial_guess(quad_params const& q, displacement offset_quad)
	{
		auto const u = q.u;
		auto const v = q.v;
		auto const w = q.w;

		auto const w_factors = shuffle(w.get(), 1, 0, 0, 1);
		auto const uv_factors = shuffle(u.get(), v.get(), 4, 1, 5, 0);
		auto const prod = w_factors*uv_factors;
		auto const sum = shuffle(prod, 0, 1, 2, 3) - shuffle(prod, 2, 3, 2, 3);

		auto const row_0 = shuffle(v.get(), sum, 1, 0, 2, 4)*geosimd::vec_t{1.0f, -1.0f, 1.0f, 1.0f};
		auto const row_1 = shuffle(u.get(), sum, 1, 0, 2, 5)*geosimd::vec_t{-1.0f, 1.0f, 1.0f, 1.0f};

		geosimd::vec_t const row_2{0.0f, 0.0f, 1.0f, 0.0f};
		geosimd::vec_t const row_3{0.0f, 0.0f, 0.0f, 1.0f};

		std::array const transform{
			row_0,
			row_1,
			row_2,
			row_3
		};
		auto const det = u[0]*v[1] - u[1]*v[0];

		geosimd::vec_t<float, 4> result{};
		for(size_t k = 0; k != 4; ++k)
		{
			result[k] = inner_product(transform[k], offset_quad.get());
		}

		return displacement{result}/det;
	}

	struct quad_to_unit_square_params
	{
		quad_params params;
		quad_fwd_matrix mat;
	};

	inline auto make_quad_to_unit_square_params(quad const& q)
	{
		auto params = make_quad_params(q);
		return quad_to_unit_square_params{
			.params = params,
			.mat = make_fwd_matrix(params)
		};
	}

	inline location map_quad_to_unit_square(quad_to_unit_square_params const& params, location loc)
	{
		auto const input_vec = loc - params.params.origin;

		auto current_offest_square = quad_to_unit_square_compute_initial_guess(params.params, input_vec);

		for(size_t k = 0; k != 8; ++k)
		{
			auto current_offset_quad = map_unit_square_to_quad_rel(current_offest_square, params.mat) - input_vec;
			auto const delta = quad_to_unit_square_compute_delta(params.params, current_offest_square, current_offset_quad);

			if(norm(delta) < 1.0e-8f)
			{
			//	printf("%zu\n", k);
				return location{} + current_offest_square;

			}

			current_offest_square -= delta;
		}

	//	printf("Did not converge %s\n", to_string(loc).c_str());
		return location{} + current_offest_square;
	}

	inline location map_quad_to_unit_square(quad const& q, location loc)
	{ return map_quad_to_unit_square(make_quad_to_unit_square_params(q), loc); }

	template<class PixelType, class Shader>
	void render_quad(quad const& q, span_2d<PixelType> output, Shader&& shader)
	{
		auto const aabb = make_aabb(q);
		auto const w = static_cast<int32_t>(output.width());
		auto const h = static_cast<int32_t>(output.height());
		auto const x_min = std::max(static_cast<int32_t>(aabb.min[0] - 1.0f), 0);
		auto const x_max = std::min(static_cast<int32_t>(aabb.max[0] + 1.0f), w);
		auto const y_min = std::max(static_cast<int32_t>(aabb.min[1] - 1.0f), 0);
		auto const y_max = std::min(static_cast<int32_t>(aabb.max[1] + 1.0f), h);
		auto const quad_params = make_quad_to_unit_square_params(q);

		for(auto y = y_min; y != y_max; ++y)
		{
			for(auto x = x_min; x != x_max; ++x)
			{
				location const loc_quad{
					static_cast<float>(x) + 0.5f,
					static_cast<float>(y) + 0.5f,
					0.0f,
				};

				auto const loc_square = map_quad_to_unit_square(quad_params, loc_quad);
				if(
					(loc_square[0] >= 0.0f && loc_square[0] <= 1.0f) &&
					(loc_square[1] >= 0.0f && loc_square[1] <= 1.0f)
				)
				{
					output(x, y) = shader(loc_square);
				}
			}
		}
	}
}

#endif
