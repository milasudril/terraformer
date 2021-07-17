#ifndef TERRAFORMER_MATHUTILS_HPP
#define TERRAFORMER_MATHUTILS_HPP

#include <cmath>
#include <vector>
#include <span>
#include <type_traits>

template<class T>
requires std::is_arithmetic_v<T> && (!std::is_const_v<T>)
using vec4_t [[gnu::vector_size(4*sizeof(T))]] = T;

template<class T, class U>
vec4_t<T> vector_cast(vec4_t<U> v)
{
	return vec4_t<T>{static_cast<T>(v[0]), static_cast<T>(v[1]), static_cast<T>(v[2]), static_cast<T>(v[3])};
}

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

struct Arc
{
	double r;
	double theta;
};

template<class T>
Arc make_arc(Size<T> size)
{
	auto const v = vec4_t<float>{1.0f, 0.5f, 0.0f, 0.0f} * vector_cast<float>(size.value());
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
class Vector
{
public:
	public:
	explicit Vector(T x, T y, T z = zero<T>()): m_value{x, y, z, zero<T>()}
	{}

	T x() const
	{ return m_value[0]; }

	T y() const
	{ return m_value[1]; }

 	T z() const
	{ return m_value[2]; }

	vec4_t<T> value() const
	{ return m_value; }

	Vector& operator+=(Vector vec)
	{
		m_value += vec.value();
		return *this;
	}

	Vector& operator-=(Vector vec)
	{
		m_value -= vec.value();
		return *this;
	}

	Vector& operator*=(T c)
	{
		m_value *= c;
		return *this;
	}

private:
	vec4_t<T> m_value;
};

template<class T>
T dot(Vector<T> a, Vector<T> b)
{
	auto temp = a.value() * b.value();
	return temp[0] + temp[1] + temp[2];
}

template<class T>
class Point
{
public:
	explicit Point(T x, T y, T z = zero<T>()): m_value{x, y, z}
	{}

	T x() const
	{ return m_value[0]; }

	T y() const
	{ return m_value[1]; }

	T z() const
	{ return m_value[2]; }

	vec4_t<T> value() const
	{ return m_value; }

	Point& operator+=(Vector<T> vec)
	{
		m_value += vec.value();
		return *this;
	}

	Point& operator-=(Vector<T> vec)
	{
		m_value -= vec.value();
		return *this;
	}

private:
	vec4_t<T> m_value;
};

template<class T>
Vector<T> operator-(Point<T> a, Point<T> b)
{
	return Vector<T>{b.value() - a.value()};
}

template<class T>
T distance_squared(Point<T> a, Point<T> b)
{
	return length_squared(a - b);
}

template<class T>
T distance(Point<T> a, Point<T> b)
{
	return length(a - b);
}

template<class T>
struct LineSegment
{
	Point<T> a;
	Point<T> b;
};

template<class T>
T length(LineSegment<T> const& l)
{
	return distance(l.a, l.b);
}

template<class T>
class Polygon
{
public:
	explicit Polygon(LineSegment<T> const& seg): m_points{seg}{}

	Polygon& append(Point<T> p)
	{
		m_points.push_back(p);
		return *this;
	}

	std::span<Point<T>> vertices() const
	{ return m_points; }

private:
	std::vector<Point<T>> m_points;
};


#endif