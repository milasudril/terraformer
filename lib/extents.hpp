#ifndef TERRAFORMER_EXTENTS_HPP
#define TERRAFORMER_EXTENTS_HPP

#include "./vec_t.hpp"
#include "./constants.hpp"

template<class T>
class Extents
{
public:
	constexpr explicit Extents(T width, T depth, T height = unity<T>()):m_value{width, depth, height, zero<T>()}{}

	constexpr T width() const { return m_value[0]; }

	constexpr T depth() const { return m_value[1]; }

	constexpr T height() const { return m_value[2]; }

	constexpr vec4_t<T> value() const
	{ return m_value; }

private:
	vec4_t<T> m_value;
};

template<class T>
constexpr T volume(Extents<T> s)
{
    return s.width() * s.depth() * s.height();
}

template<class T, class U>
constexpr bool within(T val, Extents<U> ex)
{
	return val.x() >= zero<U>() && val.x() < ex.width()
		&& val.y() >= zero<U>() && val.y() < ex.depth()
		&& val.z() >= zero<U>() && val.z() < ex.height();
}

#endif