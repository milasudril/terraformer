#ifndef TERRAFORMER_MESH_HPP
#define TERRAFORMER_MESH_HPP

#include "lib/common/array_tuple.hpp"
#include "lib/common/spaces.hpp"

#include <geosimd/triangle.hpp>

namespace terraformer
{
	using face = geosimd::indirect_triangle<uint32_t>;

	using vertex = tuple<location, direction>;

	class mesh
	{
	public:
		void push_back(vertex v)
		{ m_vertex_data.push_back(v); }

		void push_back(face const& f)
		{ m_faces.push_back(tuple{f}); }

		auto faces() const
		{ return m_faces.get<0>(); }

		auto locations() const
		{ return m_vertex_data.get<0>(); }

		auto normals() const
		{ return m_vertex_data.get<1>(); }

		bool operator==(mesh const&) const = default;
		bool operator!=(mesh const&) const = default;

	private:
		array_tuple<location, direction> m_vertex_data;
		array_tuple<face> m_faces;
	};
}

#endif