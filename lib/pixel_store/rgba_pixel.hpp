#ifndef TERRAFORMER_RGBAPIXEL_HPP
#define TERRAFORMER_RGBAPIXEL_HPP

#include <geosimd/vec_t.hpp>
#include <type_traits>
#include <utility>

namespace terraformer
{
	class rgba_pixel
	{
	public:
		using storage_type = geosimd::vec_t<float, 4>;
		using value_type = float;

		constexpr explicit rgba_pixel(storage_type value): m_value{value} {}

		constexpr explicit rgba_pixel(float r, float g, float b, float a = 1.0f)
			: m_value{r, g, b, a}
		{
		}

		constexpr rgba_pixel() = default;

		constexpr rgba_pixel(rgba_pixel const&) = default;

		constexpr rgba_pixel& red(float val)
		{
			m_value[0] = val;
			return *this;
		}

		constexpr rgba_pixel& green(float val)
		{
			m_value[1] = val;
			return *this;
		}

		constexpr rgba_pixel& blue(float val)
		{
			m_value[2] = val;
			return *this;
		}

		constexpr rgba_pixel& alpha(float val)
		{
			m_value[3] = val;
			return *this;
		}

		constexpr rgba_pixel& value(storage_type val)
		{
			m_value = val;
			return *this;
		}


		constexpr float red() const { return m_value[0]; }

		constexpr float green() const { return m_value[1]; }

		constexpr float blue() const { return m_value[2]; }

		constexpr float alpha() const { return m_value[3]; }


		constexpr auto value() const { return m_value; }

		constexpr rgba_pixel& operator+=(rgba_pixel other)
		{
			m_value += other.m_value;
			return *this;
		}

		constexpr rgba_pixel& operator-=(rgba_pixel other)
		{
			m_value -= other.m_value;
			return *this;
		}

		constexpr rgba_pixel& operator/=(rgba_pixel other)
		{
			m_value /= other.m_value;
			return *this;
		}

		constexpr rgba_pixel& operator*=(rgba_pixel other)
		{
			m_value *= other.m_value;
			return *this;
		}

		constexpr rgba_pixel& operator*=(float factor)
		{
			m_value *= factor;
			return *this;
		}

		constexpr rgba_pixel& operator/=(float factor)
		{
			m_value /= factor;
			return *this;
		}

		constexpr bool operator==(rgba_pixel const&) const = default;

		constexpr bool operator!=(rgba_pixel const&) const = default;

	private:
		storage_type m_value;
	};

	constexpr auto operator/(rgba_pixel a, float c)
	{
		return a /= c;
	}

	constexpr auto operator*(rgba_pixel a, float c)
	{
		return a *= c;
	}

	constexpr auto operator*(float c, rgba_pixel a)
	{
		return a * c;
	}

	constexpr auto operator+(rgba_pixel a, rgba_pixel b)
	{
		return a += b;
	}

	constexpr rgba_pixel black() { return rgba_pixel{0.0f, 0.0f, 0.0f}; }

	constexpr rgba_pixel red() { return rgba_pixel{1.0f, 0.0f, 0.0f}; }

	constexpr rgba_pixel green() { return rgba_pixel{0.0f, 1.0f, 0.0f}; }

	constexpr rgba_pixel blue() { return rgba_pixel{0.0f, 0.0f, 1.0f}; }

	constexpr rgba_pixel cyan() { return rgba_pixel{0.0f, 1.0f, 1.0f}; }

	constexpr rgba_pixel magenta() { return rgba_pixel{1.0f, 0.0f, 1.0f}; }

	constexpr rgba_pixel yellow() { return rgba_pixel{1.0f, 1.0f, 0.0f}; }

	constexpr rgba_pixel white() { return rgba_pixel{1.0f, 1.0f, 1.0f}; }

	constexpr auto max_color_value(rgba_pixel x)
	{ return std::max(x.red(), std::max(x.green(), x.blue())); }

	inline auto to_string(rgba_pixel const& x)
	{ return to_string(x.value()); }
}

template<>
struct std::tuple_size<terraformer::rgba_pixel>
{
	static constexpr size_t value = 4;
};

template<std::size_t Index>
struct std::tuple_element<Index, terraformer::rgba_pixel>
{
	using type = terraformer::rgba_pixel::value_type;
};
#endif