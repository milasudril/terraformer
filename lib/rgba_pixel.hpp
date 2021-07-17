//@	{"targets":[{"name":"pixel.hpp", "type":"include"}]}

#ifndef PIXELSTORE_PIXEL_HPP
#define PIXELSTORE_PIXEL_HPP

#include "./vec_t.hpp"

#include <type_traits>

class RgbaPixel
{
public:
	using StorageType = vec4_t<float>;

	constexpr explicit RgbaPixel(StorageType value): m_value{value} {}

	constexpr explicit RgbaPixel(float r, float g, float b, float a = 1.0f)
		: m_value{r, g, b, a}
	{
	}

	constexpr RgbaPixel() = default;

	constexpr RgbaPixel(RgbaPixel const&) = default;

	constexpr RgbaPixel& red(float val)
	{
		m_value[0] = val;
		return *this;
	}

	constexpr RgbaPixel& green(float val)
	{
		m_value[1] = val;
		return *this;
	}

	constexpr RgbaPixel& blue(float val)
	{
		m_value[2] = val;
		return *this;
	}

	constexpr RgbaPixel& alpha(float val)
	{
		m_value[3] = val;
		return *this;
	}

	constexpr RgbaPixel& value(StorageType val)
	{
		m_value = val;
		return *this;
	}


	constexpr float red() const { return m_value[0]; }

	constexpr float green() const { return m_value[1]; }

	constexpr float blue() const { return m_value[2]; }

	constexpr float alpha() const { return m_value[3]; }


	constexpr auto value() const { return m_value; }

	constexpr RgbaPixel& operator+=(RgbaPixel other)
	{
		m_value += other.m_value;
		return *this;
	}

	constexpr RgbaPixel& operator-=(RgbaPixel other)
	{
		m_value -= other.m_value;
		return *this;
	}

	constexpr RgbaPixel& operator/=(RgbaPixel other)
	{
		m_value /= other.m_value;
		return *this;
	}

	constexpr RgbaPixel& operator*=(RgbaPixel other)
	{
		m_value *= other.m_value;
		return *this;
	}

	constexpr RgbaPixel& operator*=(float factor)
	{
		m_value *= factor;
		return *this;
	}

	constexpr RgbaPixel& operator/=(float factor)
	{
		m_value /= factor;
		return *this;
	}

private:
	StorageType m_value;
};

constexpr auto operator/(RgbaPixel a, float c)
{
	return a /= c;
}

constexpr auto operator*(RgbaPixel a, float c)
{
	return a *= c;
}

constexpr auto operator*(float c, RgbaPixel a)
{
	return a * c;
}

constexpr RgbaPixel black() { return RgbaPixel{0.0f, 0.0f, 0.0f}; }

constexpr RgbaPixel red() { return RgbaPixel{1.0f, 0.0f, 0.0f}; }

constexpr RgbaPixel green() { return RgbaPixel{0.0f, 1.0f, 0.0f}; }

constexpr RgbaPixel blue() { return RgbaPixel{0.0f, 0.0f, 1.0f}; }

constexpr RgbaPixel cyan() { return RgbaPixel{0.0f, 1.0f, 1.0f}; }

constexpr RgbaPixel magenta() { return RgbaPixel{1.0f, 0.0f, 1.0f}; }

constexpr RgbaPixel yellow() { return RgbaPixel{1.0f, 1.0f, 0.0f}; }

constexpr RgbaPixel white() { return RgbaPixel{1.0f, 1.0f, 1.0f}; }
#endif