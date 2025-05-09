#ifndef TERRAFORMER_VALUE_ACCESSOR_HPP
#define TERRAFORMER_VALUE_ACCESSOR_HPP

#include <functional>
#include <type_traits>
#include <stdexcept>

namespace terraformer
{
	template<class T>
	auto& value_of(std::reference_wrapper<T> val)
	{ return val.get(); }

	template<class T>
	decltype(auto) value_of(T&& val)
	{ return std::forward<T>(val); }

	template<class T>
	using dereferenced_type = std::remove_cvref_t<decltype(value_of(std::declval<T>()))>;
}

#endif