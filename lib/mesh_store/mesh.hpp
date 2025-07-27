#ifndef TERRAFORMER_MESH_HPP
#define TERRAFORMER_MESH_HPP

#include "lib/array_classes/multi_array.hpp"
#include "lib/array_classes/single_array.hpp"
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
		{ m_vertex_data.push_back(get<0>(v), get<1>(v)); }

		void push_back(face const& f)
		{ m_faces.push_back(f); }

		auto faces() const
		{ return span{m_faces.begin(), m_faces.end()}; }

		auto locations() const
		{ return m_vertex_data.get<0>(); }

		auto normals() const
		{ return m_vertex_data.get<1>(); }

		bool operator==(mesh const&) const = default;
		bool operator!=(mesh const&) const = default;

	private:
		multi_array<location, direction> m_vertex_data;
		single_array<face> m_faces;
	};
}

#endif