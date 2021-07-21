#ifndef TERRAFORMER_VECT_HPP
#define TERRAFORMER_VECT_HPP

#include <type_traits>

template<class T>
requires std::is_arithmetic_v<T> && (!std::is_const_v<T>)
using vec4_t [[gnu::vector_size(4*sizeof(T))]] = T;

template<class U>
struct HasVectorSize:std::false_type{};

template<>
struct HasVectorSize<vec4_t<float>>:std::true_type{};

template<>
struct HasVectorSize<vec4_t<int>>:std::true_type{};

template<>
struct HasVectorSize<vec4_t<unsigned int>>:std::true_type{};

template<>
struct HasVectorSize<vec4_t<double>>:std::true_type{};

template<>
struct HasVectorSize<vec4_t<long long>>:std::true_type{};

template<>
struct HasVectorSize<vec4_t<unsigned long long>>:std::true_type{};

template<class T, class U>
constexpr vec4_t<T> vector_cast(U v) requires HasVectorSize<U>::value
{
	return vec4_t<T>{static_cast<T>(v[0]), static_cast<T>(v[1]), static_cast<T>(v[2]), static_cast<T>(v[3])};
}

#endif