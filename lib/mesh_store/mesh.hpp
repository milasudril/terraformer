#ifndef TERRAFORMER_MESH_HPP
#define TERRAFORMER_MESH_HPP

#include "lib/common/array_tuple.hpp"
#include "lib/common/spaces.hpp"

#include <geosimd/triangle.hpp>
#include <set>
#include <bit>

namespace terraformer
{
	using face = geosimd::indirect_triangle<uint32_t>;

	using vertex = tuple<location, direction>;

	class mesh
	{
	public:
		void push_back(vertex v)
		{ m_vertex_data.push_back(v); }

		void insert(face const& f)
		{ m_faces.insert(f); }

		auto faces() const
		{ return m_faces; }

		auto locations() const
		{ return m_vertex_data.get<0>(); }

		auto normals() const
		{ return m_vertex_data.get<1>(); }

		bool operator==(mesh const&) const = default;
		bool operator!=(mesh const&) const = default;

		auto const vertex_count() const { return std::size(m_vertex_data); }

	private:
		array_tuple<location, direction> m_vertex_data;
		struct face_compare
		{
			bool operator()(face const& a, face const& b) const
			{
				return std::bit_cast<std::array<uint32_t, 3>>(a)
					< std::bit_cast<std::array<uint32_t, 3>>(b);
			}
		};
		std::set<face, face_compare> m_faces;
	};
}

#endif