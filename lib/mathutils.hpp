#ifndef TERRAFORMER_MATHUTILS_HPP
#define TERRAFORMER_MATHUTILS_HPP

#include "./adj_algo.hpp"
#include "./vec_t.hpp"
#include "./constants.hpp"
#include "./extents.hpp"

#include <cmath>
#include <vector>
#include <span>
#include <type_traits>

struct Arc
{
	float radius;
	float angle;
};

template<class T>
Arc make_arc(Extents<T> size)
{
	auto const v = vector_cast<float>(size.value());
	auto const a = v[0];
	auto const b = v[1];
	auto const r = (b*b + a*a)/(2.0f*b);
	return Arc{r, std::atan(a/(r - b))};
}

inline double length(Arc arc)
{
	return arc.radius * arc.angle;
}

template<class T>
class Vector
{
public:
	explicit Vector(vec4_t<T> data):m_value{data}
	{m_value[3] = zero<T>();}

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

	inline Vector& normalize() requires std::floating_point<T>;

private:
	vec4_t<T> m_value;
};

template<class T>
Vector<T> operator*(T c, Vector<T> v)
{
	v*=c;
	return v;
}

template<class T>
Vector<T> operator+(Vector<T> a, Vector<T> b)
{
	a += b;
	return a;
}

template<class T>
Vector<T> operator-(Vector<T> a, Vector<T> b)
{
	a -= b;
	return a;
}

template<class T>
T dot(Vector<T> a, Vector<T> b)
{
	auto temp = a.value() * b.value();
	return temp[0] + temp[1] + temp[2];
}

template<class T>
auto length_squared(Vector<T> v)
{
	return dot(v, v);
}

template<class T>
auto length(Vector<T> v)
{
	return std::sqrt(length_squared(v));
}

template<class T>
Vector<T>& Vector<T>::normalize() requires std::floating_point<T>
{
	m_value /= length(*this);
	return *this;
}

template<std::floating_point T>
auto normalized(Vector<T> v)
{
	return v.normalize();
}

template<class T>
auto normalized(Vector<T> v)
{
	return normalized(Vector<double>{vector_cast<double>(v.value())});
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
	return Vector<T>{a.value() - b.value()};
}

template<class T>
Point<T> operator+(Point<T> a, Vector<T> b)
{
	a += b;
	return a;
}

template<class T>
Point<T> operator-(Point<T> a, Vector<T> b)
{
	a -= b;
	return a;
}

template<class T>
T distance_squared(Point<T> a, Point<T> b)
{
	return length_squared(a - b);
}

template<class T>
auto distance(Point<T> a, Point<T> b)
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
auto length(LineSegment<T> const& l)
{
	return distance(l.a, l.b);
}

template<class T>
class PolygonChain
{
public:
	explicit PolygonChain(LineSegment<T> const& seg): m_points{seg}{}

	PolygonChain& append(Point<T> p)
	{
		m_points.push_back(p);
		return *this;
	}

	std::span<Point<T>> vertices() const
	{ return m_points; }

private:
	std::vector<Point<T>> m_points;
};

template<class T>
auto& length(PolygonChain<T> const& p)
{
	auto verts = p.vertices();
	return adj_integrate(std::begin(verts), std::end(verts), [](auto a, auto b){
		return distance(a, b);
	});
}

#endif