//@	{"dependencies_extra":[{"ref":"./mesh_output.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_MESH_OUTPUT_HPP
#define TERRAFORMER_MESH_OUTPUT_HPP

#include "./mesh.hpp"
#include "lib/common/cfile_owner.hpp"

namespace terraformer
{
	void store(mesh const& mesh, FILE* output_stream, char const* object_name = nullptr);

	inline void store(mesh const& mesh, char const* filename, char const* object_name = nullptr)
	{
		auto output_file = make_output_file(filename);
		store(mesh, output_file.get(), object_name);
	}
};

#endif