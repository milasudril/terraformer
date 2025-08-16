//@	{"target": {"name":"rasterizer.o"}}

#include "./rasterizer.hpp"

#include <cassert>

terraformer::thick_curve terraformer::make_thick_curve(
	span<location const> curve,
	span<float const> curve_thickness
)
{
	assert(std::size(curve).get() == std::size(curve_thickness).get());

	terraformer::thick_curve ret{};
	auto const elems = curve_thickness.element_indices(1);
	for(auto item : elems)
	{
		auto const current_index = (item - elems.front()) - 1;

		auto compute_normal = [curve](decltype(curve)::offset_type current_index) {
			auto const loc_first = curve(current_index - 1, clamp_index{});
			auto const loc_next = curve(current_index + 1, clamp_index{});
			auto const dr = loc_next - loc_first;
			return direction{dr.rot_right_angle_z_right()};
		};

		auto const thickness_1 = curve_thickness[item];
		auto const thickness_2 = curve_thickness(current_index + 1, clamp_index{});
		auto const v1 = thickness_1*compute_normal(current_index);
		auto const v2 = thickness_2*compute_normal(current_index + 1);
		auto const loc_1 = curve[decltype(curve)::index_type{} + current_index];
		auto const loc_2 = curve[decltype(curve)::index_type{} + current_index + 1];

		auto const origin = loc_1 - v1;
		auto const lower_right = loc_1 + v1;
		auto const upper_left = loc_2 - v2;
		auto const remote = loc_2 + v2;

		auto const intersect = intersect_2d(
			geosimd::line{
				.p1 = origin,
				.p2 = lower_right
			},
			geosimd::line{
				.p1 = upper_left,
				.p2 = remote
			}
		);

		if(!intersect.has_value())
		{
			ret.data.push_back(loc_1, thickness_1);
			continue;
		}

		// It should be sufficent to check one line parameter
		if(intersect->a.get() < 0.0f || intersect->a.get() > 1.0f)
		{
			ret.data.push_back(loc_1, thickness_1);
			continue;
		}

	}


	return ret;
}