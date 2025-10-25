#include <pretty/plot.hpp>
#include <geosimd/basic_point.hpp>
#include <geosimd/unit_vector.hpp>
#include <geosimd/basic_vector.hpp>
#include <geosimd/euclidian_space.hpp>
#include <geosimd/scaling.hpp>

using geom_space = geosimd::euclidian_space<float, 3>;
using location = geom_space::location;
using displacement = geom_space::displacement;
using direction = geom_space::direction;
using scaling = geosimd::scaling<geom_space>;

struct circular_arc
{
	location origin;
	float signed_radius;
};

location get_point_on_circle(circular_arc const& arc, float theta)
{
	return arc.origin + arc.signed_radius*displacement{std::cos(theta), std::sin(theta), 0.0f};
}

circular_arc make_circular_arc(direction t_0, direction t_1, location P_0, location P_1)
{
	auto const n_0 = std::as_const(t_0).rot_right_angle_z_right();
	auto const n_1 = std::as_const(t_1).rot_right_angle_z_right();

	auto const delta_normal = n_1.get() - n_0.get();
	auto const v01 = P_1 - P_0;
	auto const r_vec = v01.apply_inv(scaling{delta_normal.get()});

	auto const r = std::abs(delta_normal[0]) > std::abs(delta_normal[1])? r_vec[0] : r_vec[1];

	return circular_arc{
		.origin = midpoint(P_0 - r*n_0, P_1 - r*n_1),
		.signed_radius = r
	};
}

void render_as_circle(circular_arc const& arc)
{
	std::array<location, 49> points;
	for(size_t k = 0; k != std::size(points); ++k)
	{
		auto const tau = 2.0f*std::numbers::pi_v<float>;
		auto const theta = tau*static_cast<float>(k)/static_cast<float>(std::size(points) - 1);
		points[k] = get_point_on_circle(arc, theta);
	}

	pretty::plot(points);
}

int main()
{
	direction const t_0{displacement{1.0f, 1.0f, 0.0f}};
	direction const t_1{displacement{1.0f, -1.0f, 0.0f}};
	location const P_0{0.0f, 0.0f, 0.0f};
	location const P_1{1.0f, 0.0f, 0.0f};

	auto const arc = make_circular_arc(t_0, t_1, P_0, P_1);
	PRETTY_PRINT_EXPR(arc.origin);
	PRETTY_PRINT_EXPR(arc.signed_radius);
	render_as_circle(arc);
}