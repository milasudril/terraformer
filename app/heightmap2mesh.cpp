//@	{"target":{"name":"heightmap2mesh.o"}}

#include "lib/pixel_store/image_io.hpp"
#include "lib/filters/heightmap_to_mesh.hpp"
#include "lib/mesh_store/mesh_output.hpp"

int main(int argc, char** argv)
{
	if(argc < 1 + 1 + 3 + 1)
	{
		puts("Usage heightmap2mesh input sx sy sz output");
		return 1;
	}

	auto const src = argv[1];
	auto const s_x = static_cast<float>(atof(argv[2]));
	auto const s_y = static_cast<float>(atof(argv[3]));
	auto const s_z = static_cast<float>(atof(argv[4]));
	auto const dest = argv[5];

	auto const mesh = create(std::type_identity<terraformer::mesh>{},
		terraformer::heightmap{
			load(std::type_identity<terraformer::grayscale_image>{}, src),
			s_x,
			s_y,
			s_z,
		});

	store(mesh, dest);

	return 0;
}
