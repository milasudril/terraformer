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
#include <algorithm>


template<class T>
class Vector
{
public:
	constexpr explicit Vector(vec4_t<T> data):m_value{data}
	{m_value[3] = zero<T>();}

	constexpr explicit Vector(T x, T y, T z = zero<T>()): m_value{x, y, z, zero<T>()}
	{}

	constexpr T x() const
	{ return m_value[0]; }

	constexpr T y() const
	{ return m_value[1]; }

 	constexpr T z() const
	{ return m_value[2]; }

	constexpr vec4_t<T> value() const
	{ return m_value; }

	constexpr Vector& operator+=(Vector vec)
	{
		m_value += vec.value();
		return *this;
	}

	constexpr Vector& operator-=(Vector vec)
	{
		m_value -= vec.value();
		return *this;
	}

	constexpr Vector& operator*=(T c)
	{
		m_value *= c;
		return *this;
	}

	constexpr inline Vector& normalize() requires std::floating_point<T>;

	constexpr Vector& scale(Vector a)
	{
		m_value *= a.value();
		return *this;
	}

private:
	vec4_t<T> m_value;
};

template<class T>
constexpr Vector<T> operator*(T c, Vector<T> v)
{
	v*=c;
	return v;
}

template<class T>
constexpr Vector<T> scale(Vector<T> a, Vector<T> b)
{
	a.scale(b);
	return a;
}

template<class T>
constexpr Vector<T> operator+(Vector<T> a, Vector<T> b)
{
	a += b;
	return a;
}

template<class T>
constexpr Vector<T> operator-(Vector<T> a, Vector<T> b)
{
	a -= b;
	return a;
}

template<class T>
constexpr T dot(Vector<T> a, Vector<T> b)
{
	auto temp = a.value() * b.value();
	return temp[0] + temp[1] + temp[2];
}

template<class T>
constexpr auto length_squared(Vector<T> v)
{
	return dot(v, v);
}

template<class T>
constexpr auto length(Vector<T> v)
{
	return std::sqrt(length_squared(v));
}

template<class T>
constexpr Vector<T>& Vector<T>::normalize() requires std::floating_point<T>
{
	m_value /= length(*this);
	return *this;
}

template<std::floating_point T>
constexpr auto normalized(Vector<T> v)
{
	return v.normalize();
}

template<class T>
constexpr auto normalized(Vector<T> v)
{
	return normalized(Vector<double>{vector_cast<double>(v.value())});
}

template<class T, class U>
constexpr auto vector_cast(Vector<U> v)
{
	return Vector<T>{vector_cast<T>(v.value())};
}

template<class T>
constexpr Vector<T> transform(Vector<T> v, Vector<T> x_vec, Vector<T> y_vec, Vector<T> z_vec)
{
	return Vector<T>{dot(v, x_vec), dot(v, y_vec), dot(v, z_vec)};
}

template<class T>
constexpr Vector<T> X{1, 0, 0};

template<class T>
constexpr Vector<T> Y{0, 1, 0};

template<class T>
constexpr Vector<T> Z{0, 0, 1};

template<class T>
class Point
{
public:
	constexpr explicit Point(vec4_t<T> v): m_value{v}
	{
		m_value[3] = unity<T>();
	}

	constexpr explicit Point(T x, T y, T z = zero<T>()): m_value{x, y, z, unity<T>()}
	{}

	constexpr T x() const
	{ return m_value[0]; }

	constexpr T y() const
	{ return m_value[1]; }

	constexpr T z() const
	{ return m_value[2]; }

	constexpr vec4_t<T> value() const
	{ return m_value; }

	constexpr Point& operator+=(Vector<T> vec)
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
constexpr Vector<T> operator-(Point<T> a, Point<T> b)
{
	return Vector<T>{a.value() - b.value()};
}

template<class T>
constexpr Point<T> operator+(Point<T> a, Vector<T> b)
{
	a += b;
	return a;
}

template<class T>
constexpr Point<T> operator-(Point<T> a, Vector<T> b)
{
	a -= b;
	return a;
}

template<class T>
constexpr T distance_squared(Point<T> a, Point<T> b)
{
	return length_squared(a - b);
}

template<class T>
constexpr auto distance(Point<T> a, Point<T> b)
{
	return length(a - b);
}

template<class T>
constexpr auto midpoint(Point<T> a, Point<T> b)
{
	return Point<T>{0.5f*(a.value() + b.value())};
}

template<class T, class U>
constexpr auto vector_cast(Point<U> v)
{
	return Point<T>{vector_cast<T>(v.value())};
}

template<class T>
constexpr Point<T> Origin{0.0f, 0.0f, 0.0f};

template<class T>
class PolygonChain
{
public:
	explicit PolygonChain(Point<T> first, Point<T> second): m_points{first, second}{}

	PolygonChain& append(Point<T> p)
	{
		m_points.push_back(p);
		return *this;
	}

	std::span<Point<T> const> vertices() const
	{ return std::span<Point<T> const>{std::data(m_points), std::size(m_points)}; }

	std::span<Point<T> > vertices()
	{ return std::span<Point<T>>{std::data(m_points), std::size(m_points)}; }

private:
	std::vector<Point<T>> m_points;
};

template<class T>
void scale(PolygonChain<T>& p, Vector<T> factor, Point<T> origin = Origin<T>)
{
	std::ranges::for_each(p.vertices(), [factor, origin](auto& val) {
		val = origin + scale(val - origin, factor);
	});
}

template<class T>
void transform(PolygonChain<T>& p, Vector<T> x_vec, Vector<T> y_vec, Vector<T> z_vec, Point<T> origin = Origin<T>)
{
	std::ranges::for_each(p.vertices(), [x_vec, y_vec, z_vec, origin](auto& val) {
		val = origin + transform(val - origin, x_vec, y_vec, z_vec);
	});
}


template<class T>
void translate(PolygonChain<T>& p, Vector<T> offset)
{
	std::ranges::for_each(p.vertices(), [offset](auto& val) {
		val += offset;
	});
}

template<class T>
auto& length(PolygonChain<T> const& p)
{
	auto verts = p.vertices();
	return adj_integrate(std::begin(verts), std::end(verts), [](auto a, auto b){
		return distance(a, b);
	});
}

template<class T>
struct LineSegment
{
	Point<T> from;
	Point<T> to;
};

template<class T>
constexpr T xy(T val)
{
	return T{val.x(), val.y()};
}

#endif