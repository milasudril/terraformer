#ifndef TERRAFORMER_MESH_3D_HPP
#define TERRAFORMER_MESH_3D_HPP

#include "lib/common/array_tuple.hpp"
#include "lib/common/spaces.hpp"

namespace terraformer
{
	struct face
	{
		uint32_t v1;
		uint32_t v2;
		uint32_t v3;
	};

	using vertex = tuple<location, displacement>;

	class mesh_3d
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

	private:
		array_tuple<location, displacement> m_vertex_data;
		array_tuple<face> m_faces;
	};
}

#endif