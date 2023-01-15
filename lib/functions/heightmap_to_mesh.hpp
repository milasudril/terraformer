#ifndef TERRAFORMER_LIB_HEIGHTMAP_TO_MESH_HPP
#define TERRAFORMER_LIB_HEIGHTMAP_TO_MESH_HPP

#include "lib/mesh_store/mesh.hpp"
#include "lib/common/utils.hpp"

namespace terraformer
{
	template<map_2d Map>
	requires(std::is_same_v<typename Map::mapped_type, float>);
	struct heightmap
	{
		Map pixels;
		float s_x;
		float s_y;
		float s_z;
	};

	template<map_2d Map>
	mesh create(std::type_identity<mesh_3d>, heighmap<Map> const& heightmap);
}

#endif