//@	{"target":{"name":"heightmap_to_mesh.test"}}

#include "./heightmap_to_mesh.hpp"

#include "lib/pixel_store/image.hpp"
#include "lib/pixel_store/image_io.hpp"
#include "lib/mesh_store/mesh_output.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_create_mesh_from_heightmap)
{
	terraformer::grayscale_image img{4, 3};
	img(0, 0) = 0.0f;
	img(1, 0) = 0.33333f;
	img(2, 0) = 0.66667f;
	img(3, 0) = 1.0f;

	img(0, 1) = 0.0f;
	img(1, 1) = 0.16667f;
	img(2, 1) = 0.33333f;
	img(3, 1) = 0.5f;

	img(0, 1) = 0.0f;
	img(1, 2) = 0.0f;
	img(2, 2) = 0.0f;
	img(3, 2) = 0.0f;

	{
		auto id_string = std::string{MAIKE_BUILDINFO_TARGETDIR "/"};
		id_string += std::to_string(MAIKE_TASKID);
		id_string += ".exr";
		store(img, id_string.c_str());
	}

	auto const mesh = create(std::type_identity<terraformer::mesh>{},
		terraformer::heightmap{img, 1.0f, 1.0f, 1.0f});

	auto const locs = mesh.locations();
	constexpr auto num_verts = 12;
	EXPECT_EQ(std::size(locs).get(), num_verts);
	std::array<float, num_verts> const x_expected{
		-1.500000f, -0.500000f, 0.500000f, 1.500000f,
		-1.500000f, -0.500000f, 0.500000f, 1.500000f,
		-1.500000f, -0.500000f, 0.500000f, 1.500000f
	};

	std::array<float, num_verts> const y_expected{
		 1.000000f,  1.000000f,  1.000000f,  1.000000f,
		 0.000000f,  0.000000f,  0.000000f,  0.000000f,
		-1.000000f, -1.000000f, -1.000000f, -1.000000f
	};

	for(auto k : locs.element_indices())
	{
		EXPECT_EQ(locs[k][0], x_expected[k.get()]);
		EXPECT_EQ(locs[k][1], y_expected[k.get()]);
		if(locs[k][0] != 1.5f && locs[k][0] != -1.5f && locs[k][1] != -1.0f && locs[k][1] != 1.0f)
		{
			EXPECT_LT(mesh.normals()[k][0], 0.0f);
			EXPECT_LT(mesh.normals()[k][1], 0.0f);
		}
		EXPECT_GT(mesh.normals()[k][2], 0.0f);
	}

	{
		auto id_string = std::string{MAIKE_BUILDINFO_TARGETDIR "/"};
		id_string += std::to_string(MAIKE_TASKID);
		id_string += ".obj";
		store(mesh, id_string.c_str());
	}
}