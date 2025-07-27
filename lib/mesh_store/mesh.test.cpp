//@	{"target":{"name":"mesh.test"}}

#include "./mesh.hpp"

#include "testfwk/testfwk.hpp"

static_assert(std::is_same_v<std::tuple_element_t<0, terraformer::vertex>, terraformer::location>);
static_assert(std::is_same_v<std::tuple_element_t<1, terraformer::vertex>, terraformer::direction>);

TESTCASE(terraformer_mesh_push_back)
{
	terraformer::mesh mesh;
	mesh.push_back(terraformer::vertex{
		terraformer::location{1.0f, 2.0f, 3.0f},
		terraformer::direction{terraformer::geom_space::x{}}
	});
	mesh.push_back(terraformer::vertex{
		terraformer::location{4.0f, 5.0f, 6.0f},
		terraformer::direction{terraformer::geom_space::y{}}
	});
	mesh.push_back(terraformer::vertex{
		terraformer::location{7.0f, 8.0f, 9.0f},
		terraformer::direction{terraformer::geom_space::z{}}
	});

	mesh.push_back(terraformer::face{0, 1, 2});

	EXPECT_EQ(std::size(mesh.faces()).get(), 1);
	EXPECT_EQ(std::size(mesh.locations()).get(), 3);
	EXPECT_EQ(std::size(mesh.normals()).get(), 3);

	auto const faces = mesh.faces();
	EXPECT_EQ(faces.front().v1, 0);
	EXPECT_EQ(faces.front().v2, 1);
	EXPECT_EQ(faces.front().v3, 2);

	auto const locations = mesh.locations();
	auto const loc_elems =mesh.locations().element_indices();
	EXPECT_EQ(locations[loc_elems.begin() + size_t{0}], (terraformer::location{1.0f, 2.0f, 3.0f}));
	EXPECT_EQ(locations[loc_elems.begin() + size_t{1}], (terraformer::location{4.0f, 5.0f, 6.0f}));
	EXPECT_EQ(locations[loc_elems.begin() + size_t{2}], (terraformer::location{7.0f, 8.0f, 9.0f}));

	auto const normals = mesh.normals();
	auto const normal_elems =mesh.normals().element_indices();
	EXPECT_EQ(normals[normal_elems.begin() + 0], terraformer::direction{terraformer::geom_space::x{}});
	EXPECT_EQ(normals[normal_elems.begin() + 1], terraformer::direction{terraformer::geom_space::y{}});
	EXPECT_EQ(normals[normal_elems.begin() + 2], terraformer::direction{terraformer::geom_space::z{}});
}