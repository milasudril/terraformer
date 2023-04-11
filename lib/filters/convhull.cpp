//@	{
//@	"target":{
//@		"name":"convhull.o",
//@		"dependencies":[
//@			{"ref":"CGAL", "origin":"system"},
//@			{"ref":"gmp", "origin":"pkg-config"}
//@		]}
//@	}

#include "./convhull.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

#include <geosimd/triangle.hpp>
#include <random>

namespace
{
	struct loc_2d
	{
		float x;
		float y;
	};

	bool is_ccw(loc_2d prevprev, loc_2d prev, loc_2d current)
	{
		auto const dx_1 = prev.x - prevprev.x;
		auto const dy_1 = prev.y - prevprev.y;
		auto const dx_2 = current.x - prev.x;
		auto const dy_2 = current.y - prev.y;

		return dy_2/dx_2 > dy_1/dx_1;
	}
}

std::vector<float> terraformer::convhull(std::span<float const> values)
{
	std::vector<loc_2d> buffer;
	for(size_t k = 0; k != std::size(values); ++k)
	{
		loc_2d const val{static_cast<float>(k), values[k]};

		while(std::size(buffer) >= 2)
		{
			auto const prevprev = *(std::end(buffer) - 2);
			auto const prev = *(std::end(buffer) - 1);
			if(!is_ccw(prevprev, prev, val))
			{ break; }

			buffer.pop_back();
		}

		buffer.push_back(val);
	}

	std::vector<float> ret(std::size(values));
	auto loc_prev = buffer[0];
	for(size_t k = 1; k != std::size(buffer); ++k)
	{
		auto const loc_current = buffer[k];
		auto const dx = loc_current.x - loc_prev.x;
		auto const dy = loc_current.y - loc_prev.y;
		auto const slope = dy/dx;
		for(size_t l = 0; l != static_cast<size_t>(dx); ++l)
		{
			auto const y = loc_prev.y + static_cast<float>(l)*slope;
			ret[static_cast<size_t>(loc_prev.x) + l] = y;
		}
		loc_prev = loc_current;
	}
	ret.back() = values.back();
	return ret;
}

namespace
{
	using cgal_kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
	using cgal_polyhedron = CGAL::Polyhedron_3<cgal_kernel>;
	using cgal_point = cgal_kernel::Point_3;
	using cgal_mesh = CGAL::Surface_mesh<cgal_point>;
}

terraformer::basic_image<float> terraformer::convhull(span_2d<float const> values)
{
	auto const w = values.width();
	auto const h = values.height();

	cgal_mesh convhull;
	{
		auto const points = to_location_array<cgal_point>(values);
		CGAL::convex_hull_3(std::begin(points), std::end(points), convhull);
		CGAL::Polygon_mesh_processing::triangulate_faces(convhull);
	}

	std::vector<location> vertices;
	std::ranges::transform(convhull.vertices(), std::back_inserter(vertices), [&convhull](auto vi){
		auto const p = convhull.point(vi);
		return location{static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z())};
	});

	std::vector<geosimd::indirect_triangle<uint32_t>> faces;
	std::ranges::transform(convhull.faces(), std::back_inserter(faces), [&convhull](auto fi){
		auto hf = convhull.halfedge(fi);
		std::array<uint32_t, 3> verts{};
		size_t k = 0;
		for(auto edge_index : halfedges_around_face(hf, convhull))
		{
			assert(k != 3);
			verts[k] = target(edge_index, convhull);
			++k;
		}

		return geosimd::indirect_triangle<uint32_t>{verts[0], verts[1], verts[2]};

	});

	basic_image<float> ret{w, h};
	for(auto const& face : faces)
	{	auto const T = geosimd::resolve(face, vertices);

		project_from_above(T, [](location loc, auto pixels){
			auto const x = static_cast<uint32_t>(loc[0]);
			auto const y = static_cast<uint32_t>(loc[1]);
			pixels(x, y) = std::max(pixels(x, y), loc[2]);
		}, ret.pixels());
	}

	return ret;
}