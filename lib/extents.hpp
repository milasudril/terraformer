#ifndef TERRAFORMER_EXTENTS_HPP
#define TERRAFORMER_EXTENTS_HPP

#include "./vec_t.hpp"
#include "./constants.hpp"

template<class T>
class Size
{
public:
	explicit Size(T width, T depth, T height = unity<T>()):m_value{width, depth, height, zero<T>()}{}

	T width() const { return m_value[0]; }

	T depth() const { return m_value[1]; }

	T height() const { return m_value[2]; }

	vec4_t<T> value() const
	{ return m_value; }

private:
	vec4_t<T> m_value;
};

template<class T>
T volume(Size<T> s)
{
    return s.width() * s.depth() * s.height();
}

#endif