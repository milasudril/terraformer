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
	auto& value_of(T& val)
	{ return val; }

	template<class T>
	decltype(auto) value_of(T&& val)
	{ return std::forward<T>(val); }

	template<class T>
	requires std::is_convertible_v<T, bool> && requires(T x){
		{*x};
	}
	decltype(auto) value_of(T&& ptr)
	{
		if(!ptr)
		{ throw std::runtime_error{"Null pointer dereference"}; }
		return *std::forward<T>(ptr);
	}
}

#endif