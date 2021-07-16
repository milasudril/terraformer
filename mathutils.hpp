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

template<class T>
class Vector2d
{
public:
	public:
	explicit Vector2d(T x, T y): m_value{x, y}
	{}

	T x() const
	{ return m_value[0]; }

	T y() const
	{ return m_value[1]; }

	vec2_t<T> value() const
	{
		return m_value;
	}

	Vector2d& operator+=(Vector2d vec)
	{
		m_value += vec.value();
		return *this;
	}

	Vector2d& operator-=(Vector2d vec)
	{
		m_value -= vec.value();
		return *this;
	}

	Vector2d& operator*=(T c)
	{
		m_value *= c;
		return *this;
	}

private:
	vec2_t<T> m_value;
};

template<class T>
T dot(Vector2d<T> a, Vector2d<T> b)
{
	auto temp = a.value() * b.value();
	return temp[0] + temp[1];
}

template<class T>
class Point2d
{
public:
	explicit Point2d(T x, T y): m_value{x, y}
	{}

	T x() const
	{ return m_value[0]; }

	T y() const
	{ return m_value[1]; }

	vec2_t<T> value() const
	{
		return m_value;
	}

	Point2d& operator+=(Vector2d<T> vec)
	{
		m_value += vec.value();
		return *this;
	}

	Point2d& operator-=(Vector2d<T> vec)
	{
		m_value -= vec.value();
		return *this;
	}

private:
	vec2_t<T> m_value;
};

template<class T>
Vector2d<T> operator-(Point2d<T> a, Point2d<T> b)
{
	return Vector2d<T>{b.value() - a.value()};
}

template<class T>
T distance_squared(Point2d<T> a, Point2d<T> b)
{
	auto diff = a - b;
	return dot(diff, diff);
}


#endif