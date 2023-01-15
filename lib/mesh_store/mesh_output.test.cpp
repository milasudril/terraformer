//@	{"target":{"name":"mesh_output.test"}}

#include "./mesh_output.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_mesh_store_store_mesh)
{
	terraformer::mesh_3d mesh;
	constexpr auto num_verts = 4;

	std::array<terraformer::location, num_verts> locs
	{
		terraformer::location{ 0.000000f,  1.154701f, 0.000000f},
		terraformer::location{ 0.000000f,  0.000000f, 1.000000f},
		terraformer::location{ 1.000000f, -0.577350f, 0.000000f},
		terraformer::location{-1.000000f, -0.577350f, 0.000000f}
	};

	std::array<terraformer::displacement, num_verts> normals
	{
		terraformer::displacement{-0.0000f,  0.9687f, -0.2481f},
		terraformer::displacement{-0.0000f, -0.0000f,  1.0000f},
		terraformer::displacement{ 0.8389f, -0.4844f, -0.2481f},
		terraformer::displacement{-0.8389f, -0.4844f, -0.2481f}
	};

	for(size_t k = 0; k != num_verts; ++k)
	{
		mesh.push_back(terraformer::vertex{
			locs[k],
			terraformer::direction{normals[k]}
		});
	}

	constexpr auto num_faces = 4;
	std::array<terraformer::face, num_faces> faces
	{
		terraformer::face{0, 1, 2},
		terraformer::face{2, 1, 3},
		terraformer::face{3, 1, 0},
		terraformer::face{2, 3, 0}
	};

	for(size_t k = 0; k != num_faces; ++k)
	{
		mesh.push_back(faces[k]);
	}

	auto id_string = std::string{MAIKE_BUILDINFO_TARGETDIR "/"};
	id_string += std::to_string(MAIKE_TASKID);
	id_string += ".obj";
	store(mesh, id_string.c_str(), "simplex");
}