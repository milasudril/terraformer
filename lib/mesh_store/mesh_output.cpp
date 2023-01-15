//@	{"target":{"name":"mesh_output.o"}}

#include "./mesh_output.hpp"

#include <charconv>

namespace
{
	template<class T>
	auto write_vector(char* dest, char* end, T value)
	{
		std::to_chars_result res{};
		res.ptr = dest;

		res = std::to_chars(res.ptr, end, value[0], std::chars_format::scientific);
		*res.ptr = ' ';
		++res.ptr;

		res = std::to_chars(res.ptr, end, value[1], std::chars_format::scientific);
		*res.ptr = ' ';
		++res.ptr;

		res = std::to_chars(res.ptr, end, value[2], std::chars_format::scientific);
		*res.ptr = '\n';
		++res.ptr;

		return res;
	}

}


void terraformer::store(mesh_3d const& mesh, FILE* file, char const* object_name)
{
	if(object_name != nullptr)
	{
		fprintf(file, "o %s\n", object_name);
	}

	for(auto location : mesh.locations())
	{
		std::array<char, 64> buffer{'v', ' '};

		std::to_chars_result res{};
		res.ptr = std::data(buffer) + 2;

		res = write_vector(res.ptr, std::end(buffer), location);

		fwrite(std::data(buffer), sizeof(char), res.ptr - std::data(buffer), file);
	}

	for(auto normal : mesh.normals())
	{
		std::array<char, 64> buffer{'v', 'n', ' '};

		std::to_chars_result res{};
		res.ptr = std::data(buffer) + 3;

		res = write_vector(res.ptr, std::end(buffer), normal);

		fwrite(std::data(buffer), sizeof(char), res.ptr - std::data(buffer), file);
	}

	fputs("s 1\n", file);

	for(auto face : mesh.faces())
	{
		fprintf(file, "f %d//%d %d//%d %d//%d\n",
			face.v1 + 1, face.v1 + 1,
			face.v2 + 1, face.v2 + 1,
			face.v3 + 1, face.v3 + 1);
	}
}