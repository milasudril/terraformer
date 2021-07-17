#ifndef TERRAFORMER_CONSTANTS_HPP
#define TERRAFORMER_CONSTANTS_HPP

template<class T>
constexpr T unity()
{
	return static_cast<T>(1);
}

template<class T>
constexpr T zero()
{
	return static_cast<T>(0);
}

#endif