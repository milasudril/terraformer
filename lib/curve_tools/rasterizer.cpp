//@	{"target": {"name":"rasterizer.o"}}

#include "./rasterizer.hpp"
#include "lib/array_classes/span.hpp"
#include "lib/common/spaces.hpp"
#include "lib/common/span_2d.hpp"
#include "lib/math_utils/cubic_spline.hpp"
#include "lib/math_utils/polynomial.hpp"

#include <cassert>
#include <geosimd/basic_vector.hpp>


float terraformer::curve_length(
	polynomial<displacement, 3> const& curve,
	float t_start,
	float t_end,
	size_t seg_count
)
{
	auto loc_start = location{} + curve(t_start);
	auto const dt = (t_end - t_start)/static_cast<float>(seg_count);
	auto ret = 0.0f;
	for(size_t k = 1; k != seg_count + 1; ++k)
	{
		auto const t =  t_start + static_cast<float>(k)*dt;
		auto const loc = location{} + curve(t);
		ret += distance(loc, loc_start);
		loc_start = loc;
	};

	return ret;
}

terraformer::spline_with_length
terraformer::make_spline_with_lengths(span<location const> curve)
{
	if(std::size(curve).get() < 2)
	{ return spline_with_length{}; }

	spline_with_length ret{};
	auto const indices = curve.element_indices(1);
	cubic_spline_control_point prev_control_point{
		.y = curve.front(),
		.ddx = curve[indices.front()] - curve.front()
	};

	for(auto k = indices.front(); k != indices.back(); ++k)
	{
		cubic_spline_control_point const control_point{
			.y = curve[k],
			.ddx = 0.5f*(curve[k + 1] - curve[k - 1])
		};

		auto const p = make_polynomial(prev_control_point, control_point);
		ret.push_back(
			p,
			line_segment{
				.from = curve[k - 1],
				.to = curve[k]
			},
			curve_length(p, 0.0f, 1.0f, 3)
		);
		prev_control_point = control_point;
	}

	auto const p = make_polynomial(
		prev_control_point,
		cubic_spline_control_point{
			.y = curve.back(),
			.ddx = curve.back() - curve[indices.back() - 1]
		}
	);
	ret.push_back(
		p,
		line_segment{
			.from = curve[indices.back() - 1],
			.to = curve.back()
		},
		curve_length(p, 0.0f, 1.0f, 3)
	);

	return ret;
}

terraformer::single_array<terraformer::polynomial<terraformer::displacement, 3>>
terraformer::make_spline(span<location const> curve)
{
	if(std::size(curve).get() < 2)
	{ return single_array<polynomial<displacement, 3>>{}; }

	single_array ret{array_size<polynomial<displacement, 3>>{std::size(curve).get() - 1}};

	auto const indices = curve.element_indices(1);
	cubic_spline_control_point prev_control_point{
		.y = curve.front(),
		.ddx = curve[indices.front()] - curve.front()
	};

	for(auto k = indices.front(); k != indices.back(); ++k)
	{
		cubic_spline_control_point const control_point{
			.y = curve[k],
			.ddx = 0.5f*(curve[k + 1] - curve[k - 1])
		};

		ret[decltype(ret)::index_type{(k - 1).get()}] = make_polynomial(prev_control_point, control_point);
		prev_control_point = control_point;
	}

	ret.back() = make_polynomial(
		prev_control_point,
		cubic_spline_control_point{
			.y = curve.back(),
			.ddx = curve.back() - curve[indices.back() - 1]
		}
	);

	return ret;
}

namespace
{
	terraformer::location
	eval_poly(
		float t,
		terraformer::polynomial<terraformer::displacement, 3> const& curve,
		terraformer::line_segment input_seg
	)
	{
		if(t > 0.0f && t < 1.0f) [[unlikely]]
		{ return terraformer::location{} + curve(t); }

		if(t <= 0.0f)
		{ return input_seg.from; }

		if(t >= 1.0f)
		{ return input_seg.to; }

		abort();
	}
}

terraformer::closest_point_info
terraformer::find_closest_point(polynomial<displacement, 3> const& curve, line_segment input_seg, location loc)
{
	// Initial guess based on a straight line
	auto const p = input_seg.to;
	auto const p_0 = input_seg.from;
	auto const curve_vector = p - p_0;
	auto const seg_length = norm(curve_vector);
	auto const p0_to_loc = loc - p_0;
	auto const tangent_vector = curve_vector/seg_length;
	auto const proj = inner_product(tangent_vector, p0_to_loc)/seg_length;

	auto t = proj;
	auto const distance_squared = take_square_of(
		curve - polynomial{loc - location{}},
		[](auto a, auto b) {
			return inner_product(a, b);
		}
	);
	auto const should_be_zero = distance_squared.derivative();
	auto const should_be_zero_deriv = should_be_zero.derivative();
	for(size_t k = 0; k != 4; ++k)
	{ t = t - should_be_zero(t)/should_be_zero_deriv(t); }

	auto const p_intersect = eval_poly(t, curve, input_seg);
	auto const d_new = distance(p_intersect, loc);

	return closest_point_info{
		.curve_parameter = t,
		.distance = d_new
	};
}

terraformer::closest_point_info
terraformer::find_closest_point(spline_with_length const& curve, location loc)
{
	if(curve.empty())
	{ return closest_point_info{}; }

	auto const polys = curve.polynomials();
	auto const lengths = curve.curve_lengths();
	auto const segs = curve.line_segments();

	struct current_best_point
	{
		decltype(polys)::index_type point_index;
		closest_point_info point_info;
		float running_distance;
	};

	current_best_point best_point{
		.point_index = curve.element_indices().front(),
		.point_info = find_closest_point(polys.front(), segs.front(), loc),
		.running_distance = 0.0f
	};

	auto running_distance = lengths.front();
	for(auto k : curve.element_indices(1))
	{
		auto const next = find_closest_point(polys[k], segs[k], loc);
		if(next.distance < best_point.point_info.distance)
		{
			best_point.point_index = k;
			best_point.point_info = next;
			best_point.running_distance = running_distance;
		}

		running_distance += lengths[k];
	}

	static constexpr size_t curve_segs = 3;
	return closest_point_info{
		.curve_parameter = best_point.running_distance
			+ curve_length(
				polys[best_point.point_index],
				0.0f,
				best_point.point_info.curve_parameter,
				curve_segs
			),
		.distance = best_point.point_info.distance,
	};
}

terraformer::closest_point_info
terraformer::find_closest_point(span<location const> curve, location loc)
{
	if(curve.empty())
	{ return closest_point_info{}; }

	auto p_0 = curve.front();
	closest_point_info ret{
		.curve_parameter = 0.0f,
		.distance = distance(p_0, loc)
	};

	auto running_distance = 0.0f;

	for(auto k : curve.element_indices(1))
	{
		auto const p = curve[k];
		auto const curve_vector = p - p_0;
		auto const seg_length = norm(curve_vector);
		auto const p0_to_loc = loc - p_0;
		auto const t = curve_vector/seg_length;
		auto const proj = std::clamp(inner_product(t, p0_to_loc), 0.0f, seg_length);
		auto const p_intersect = p_0 + proj*t;
		auto const d_new = distance(p_intersect, loc);
		if(d_new < ret.distance)
		{
			ret.curve_parameter = running_distance + proj;
			ret.distance = d_new;
		}
		running_distance += distance(p, p_0);
		p_0 = p;
	}

	return ret;
}

namespace
{
	void draw_circle(terraformer::span_2d<bool> output, terraformer::location loc, float r)
	{
		auto const x_0 = loc[0];
		auto const y_0 = loc[1];
		auto const x_min = std::clamp(
			static_cast<int32_t>(x_0 - r + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_min = std::clamp(
			static_cast<int32_t>(y_0 - r + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		auto const x_max = std::clamp(
			static_cast<int32_t>(x_0 + r + 0.5f),
			0,
			static_cast<int32_t>(output.width())
		);

		auto const y_max = std::clamp(
			static_cast<int32_t>(y_0 + r + 0.5f),
			0,
			static_cast<int32_t>(output.height())
		);

		auto const r2 = r*r;
		for(int32_t y = y_min; y != y_max; ++y)
		{
			for(int32_t x = x_min; x != x_max; ++x)
			{
				auto const v = (
					  (terraformer::location{static_cast<float>(x), static_cast<float>(y), 0.0f} - loc)
					+ terraformer::displacement{0.5f, 0.5f, 0.0f}
				);

				if(geosimd::norm_squared(v) <= r2)
				{ output(x, y) = true; }
			}
		}
	}
}

void terraformer::render_mask(
	span_2d<bool> output,
	span<location const> curve,
	curve_render_mask_descriptor params
)
{
	visit_pixels(
		curve,
		params.pixel_size,
		[output, r = params.radius/params.pixel_size](location origin, auto...){
			draw_circle(output, origin, r);
		}
	);
}