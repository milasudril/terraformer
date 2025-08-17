//@	{"target": {"name":"rasterizer.o"}}

#include "./rasterizer.hpp"
#include "lib/array_classes/span.hpp"

#include <cassert>

terraformer::thick_curve terraformer::make_thick_curve(
	span<location const> curve,
	span<float const> curve_thickness
)
{
	assert(std::size(curve).get() == std::size(curve_thickness).get());

	terraformer::thick_curve ret{};
	if(std::size(curve).get() < 2)
	{ return ret; }

	auto current_index = curve.element_indices().front() - curve.element_indices().front();
	thick_curve::vertex saved_vertex{
		.loc = curve.front(),
		.normal = direction{
			(curve(current_index + 1, clamp_index{}) - curve(current_index - 1, clamp_index{})).rot_right_angle_z_right()
		},
		.thickness = curve_thickness.front(),
		.running_length = 0.0f
	};
	ret.data.push_back(
		saved_vertex.loc,
		saved_vertex.normal,
		saved_vertex.thickness,
		saved_vertex.running_length
	);
	++current_index;

	using offset_type = decltype(curve)::offset_type;
	static_assert(std::is_same_v<decltype(current_index), offset_type>);

	for(;
		current_index != static_cast<offset_type>(std::size(curve).get());
		++current_index
	)
	{
		auto const prev_loc = saved_vertex.loc;
		auto const current_loc = curve(current_index, clamp_index{});
		auto const next_loc = curve(current_index + 1, clamp_index{});

		auto const prev_normal = saved_vertex.normal;
		auto const current_normal = direction{(next_loc - prev_loc).rot_right_angle_z_right()};

		auto const prev_thickness = saved_vertex.thickness;
		auto const current_thickness = curve_thickness(current_index, clamp_index{});

		auto const prev_v = prev_thickness*prev_normal;
		auto const current_v = current_thickness*current_normal;

		auto const origin = prev_loc - prev_v;
		auto const lower_right = prev_loc + prev_v;
		auto const upper_left = current_loc - current_v;
		auto const remote = current_loc + current_v;

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

		if(intersect.has_value() && (intersect->a.get() >= 0.0f && intersect->a.get() <= 1.0f))
		{
			auto const new_loc = midpoint(prev_loc, current_loc);
			auto const new_thickness = 0.5f*(prev_thickness + current_thickness);
			auto const saved_length =ret.running_lengths().empty()? 0.0f : ret.running_lengths().back();
			ret.data.pop_back();

			auto const saved_locations = ret.locations();
			auto const prev_saved_loc = saved_locations.empty()? curve.front() : saved_locations.back();

			saved_vertex = thick_curve::vertex{
				.loc = new_loc,
				.normal = direction{(next_loc - prev_saved_loc).rot_right_angle_z_right()},
				.thickness = new_thickness,
				.running_length = (ret.curve_length - saved_length)
					+ (saved_locations.empty()? 0.0f : distance(new_loc, prev_saved_loc))
			};
		}
		else {
			saved_vertex = thick_curve::vertex{
				.loc = current_loc,
				.normal = current_normal,
				.thickness = current_thickness,
				.running_length = ret.curve_length + distance(current_loc, prev_loc)
			};
		}
		ret.data.push_back(
			saved_vertex.loc,
			saved_vertex.normal,
			saved_vertex.thickness,
			saved_vertex.running_length
		);
		ret.curve_length = saved_vertex.running_length;
	}

	if(std::size(ret.data).get() == 1)
	{
		ret.data.clear();
		ret.curve_length = 0.0f;
	}

	return ret;
}