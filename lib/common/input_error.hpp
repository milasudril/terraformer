#ifndef TERRAFORMER_INPUT_ERROR_HPP
#define TERRAFORMER_INPUT_ERROR_HPP

#include <stdexcept>

namespace terraformer
{
	class input_error:public std::runtime_error
	{
	public:
		explicit input_error(std::string str):std::runtime_error{std::move(str)}{}
	};
}

#endif