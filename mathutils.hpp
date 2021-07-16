#ifndef TERRAFORMER_MATHUTILS_HPP
#define TERRAFORMER_MATHUTILS_HPP

#include <cmath>

template<class T>
using vec2_t [[gnu::vector_size(2*sizeof(T))]] = T;

template<class T, class U>
vec2_t<T> vector_cast(vec2_t<U> v)
{
	return vec2_t<T>{static_cast<T>(v[0]), static_cast<T>(v[1])};
}

template<class T>
class Size2d
{
    public:
        explicit Size2d(T width, T height):m_value{width, height}{}

        T width() const { return m_value[0]; }

        T height() const { return m_value[1]; }

        vec2_t<T> value() const
        { return m_value; }

    private:
    vec2_t<T> m_value;
};

template<class T>
T area(Size2d<T> s)
{
    return s.width() * s.height();
}

struct Arc
{
	double r;
	double theta;
};

template<class T>
Arc make_arc(Size2d<T> size)
{
	auto const v = vec2_t<double>{1.0, 0.5} * vector_cast<double>(size.value());
	auto const a = v[0];
	auto const b = v[1];
	auto const r = (b*b + a*a)/(2.0*b);
	return Arc{r, std::atan(a/(r - b))};
}

inline double length(Arc arc)
{
	return arc.r * arc.theta;
}

#endif