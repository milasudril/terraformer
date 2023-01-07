#ifndef TERRAFORMER_LIB_UTILS_HPP
#define TERRAFORMER_LIB_UTILS_HPP

namespace terraformer
{
	template<class T>
	constexpr T const* as_ptr_to_const(T* ptr)
	{
		return ptr;
	}
}

#endif