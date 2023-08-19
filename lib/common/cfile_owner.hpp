#ifndef TERRAFORMER_CFILE_OWNER_HPP
#define TERRAFORMER_CFILE_OWNER_HPP

#include <memory>
#include <cstdio>
#include <stdexcept>

namespace terraformer
{
	struct cfile_deleter
	{
		void operator()(FILE* f)
		{
			fclose(f);
		}
	};


	inline auto make_output_file(char const* filename)
	{
		auto ret = std::unique_ptr<FILE, cfile_deleter>(fopen(filename, "wb"));

		if(ret == nullptr)
		{
			throw std::runtime_error{"Failed to open output file"};
		}

		return ret;
	}
}

#endif