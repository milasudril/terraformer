#ifndef TERRAFORMER_LIB_UTILS_HPP
#define TERRAFORMER_LIB_UTILS_HPP

#include <type_traits>

namespace terraformer
{
	template<class T, class U>
	concept same_as_unqual = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

	template<class T>
	constexpr T const* as_ptr_to_const(T* ptr)
	{
		return ptr;
	}
}

#endif