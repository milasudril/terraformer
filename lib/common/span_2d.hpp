#ifndef TERRAFORMER_SPAN2D_HPP
#define TERRAFORMER_SPAN2D_HPP

#include "./spaces.hpp"

#include <functional>
#include <cstdint>
#include <type_traits>
#include <cstddef>
#include <cmath>
#include <limits>
#include <algorithm>

namespace terraformer
{
	struct span_2d_extents
	{
		uint32_t width;
		uint32_t height;

		constexpr bool operator==(span_2d_extents const&) const = default;
		constexpr bool operator!=(span_2d_extents const&) const = default;
	};

	inline auto diagonal(span_2d_extents extents)
	{
		auto const w = static_cast<double>(extents.width);
		auto const h = static_cast<double>(extents.height);
		return std::sqrt(w*w + h*h);
	}

	template<class T>
	class span_2d
	{
	public:
		using IndexType = uint32_t;

		using mapped_type = T;

		constexpr explicit span_2d(): span_2d{0u, 0u, nullptr} {}

		constexpr explicit span_2d(IndexType w, IndexType h, T* ptr):
			m_width{w},
			m_height{h},
			m_ptr{ptr}
		{}

		template<class U>
		constexpr explicit(!std::is_same_v<std::decay_t<U>, std::decay_t<T>>)
			span_2d(span_2d<U> other)
			: m_width{other.width()}
			, m_height{other.height()}
			, m_ptr{other.data()}
		{
		}

		constexpr auto begin() const { return m_ptr; }

		constexpr auto end() const
		{ return m_ptr + static_cast<size_t>(m_width)*static_cast<size_t>(m_height); }

		constexpr auto data() const { return begin(); }

		constexpr auto width() const { return m_width; }

		constexpr auto height() const { return m_height; }

		constexpr auto extents() const { return span_2d_extents{m_width, m_height}; }

		constexpr T& operator()(IndexType x, IndexType y) const
		{	return *(m_ptr + y * width() + x); }

	private:
		IndexType m_width;
		IndexType m_height;
		T* m_ptr;
	};

	template<class T>
	bool inside(span_2d<T const> span, float x, float y)
	{
		return (x >= 0.0f && x < static_cast<float>(span.width()))
			&& (y >= 0.0f && y < static_cast<float>(span.height()));
	}

	struct pixel_coordinates
	{
		uint32_t x;
		uint32_t y;

		bool operator==(pixel_coordinates const&) const = default;
		bool operator!=(pixel_coordinates const&) const = default;
	};

	struct scanline_range
	{
		uint32_t begin;
		uint32_t end;
	};

	template<class T, class Func>
	void for_each(span_2d<T> span, Func&& f)
	{
		using IndexType = typename span_2d<T>::IndexType;
		for(IndexType row = 0; row != span.height(); ++row)
		{
			for(IndexType col = 0; col != span.width(); ++col)
			{
				f(col, row, span(col, row));
			}
		}
	}

	template<class T, class Cmp = std::less<T>>
	pixel_coordinates max_element(span_2d<T> span, Cmp&& cmp = std::less<T>{})
	{
		using IndexType = typename span_2d<T>::IndexType;
		pixel_coordinates ret{static_cast<IndexType>(0), static_cast<IndexType>(0)};
		auto max = span(ret.x, ret.y);
		for(IndexType row = 0; row != span.height(); ++row)
		{
			for(IndexType col = 0; col != span.width(); ++col)
			{
				auto const val = span(col, row);
				if(cmp(max, val))
				{
					max = val;
					ret = pixel_coordinates{col, row};
				}
			}
		}
		return ret;
	}

	template<class T, class Cmp = std::less<T>>
	std::ranges::min_max_result<pixel_coordinates>
	minmax_element(span_2d<T> span, Cmp&& cmp = std::less<T>{})
	{
		using IndexType = typename span_2d<T>::IndexType;
		pixel_coordinates minloc{static_cast<IndexType>(0), static_cast<IndexType>(0)};
		pixel_coordinates maxloc{static_cast<IndexType>(0), static_cast<IndexType>(0)};
		auto min = span(minloc.x, minloc.y);
		auto max = span(maxloc.x, maxloc.y);
		for(IndexType row = 0; row != span.height(); ++row)
		{
			for(IndexType col = 0; col != span.width(); ++col)
			{
				auto const val = span(col, row);
				if(cmp(val, min))
				{
					min = val;
					minloc = pixel_coordinates{col, row};
				}

				if(cmp(max, val))
				{
					max = val;
					maxloc = pixel_coordinates{col, row};
				}
			}
		}
		return std::ranges::min_max_result{
			.min = minloc,
			.max = maxloc
		};
	}

	template<class In, class Out, class Func>
	void transform(span_2d<In const> in, span_2d<Out> out, Func&& f)
	{
		using IndexType = typename span_2d<Out>::IndexType;

		for(IndexType row = 0; row != out.height(); ++row)
		{
			for(IndexType col = 0; col != out.width(); ++col)
			{
				out(col, row) = f(in(col, row));
			}
		}
	}

	template<class In1, class In2, class Out, class Func>
	void transform(span_2d<In1 const> in_1, span_2d<In2 const> in_2, span_2d<Out> out, Func&& f)
	{
		using IndexType = typename span_2d<Out>::IndexType;

		for(IndexType row = 0; row != out.height(); ++row)
		{
			for(IndexType col = 0; col != out.width(); ++col)
			{
 				out(col, row) = f(in_1(col, row), in_2(col, row));
			}
		}
	}

	template<class T, class Func>
	void generate(span_2d<T> out, Func&& f)
	{
		using IndexType = typename span_2d<T>::IndexType;
		for(IndexType row = 0; row != out.height(); ++row)
		{
			for(IndexType col = 0; col != out.width(); ++col)
			{
				out(col, row) = f(col, row);
			}
		}
	}

	template<class Func>
	auto generate_minmax(span_2d<float> out, Func&& f)
	{
		std::ranges::min_max_result<float> ret{
			.min =  std::numeric_limits<float>::infinity(),
			.max = -std::numeric_limits<float>::infinity()
		};

		for(uint32_t y = 0; y != out.height(); ++y)
		{
			for(uint32_t x = 0; x != out.width(); ++x)
			{
				auto value = f(x, y);
				ret.min = std::min(value, ret.min);
				ret.max = std::max(value, ret.max);
				out(x, y) = value;
			}
		}
		return ret;
	}

	inline auto to_location(span_2d<float const> span, uint32_t x, uint32_t y)
	{
		return location{static_cast<float>(x), static_cast<float>(y), span(x, y)};
	}

	inline auto to_location(span_2d<float const> span, pixel_coordinates loc)
	{
		return to_location(span, loc.x, loc.y);
	}

	struct always_true
	{
		template<class ... T>
		constexpr bool operator()(T&&...) const { return true; }
	};

	template<class PointType, class Filter = always_true>
	inline auto to_location_array(span_2d<float const> span, Filter&& filter = always_true{})
	{
		std::vector<PointType> ret;
		ret.reserve(span.width() * span.height());

		for_each(span, [f = std::forward<Filter>(filter), &ret](uint32_t x, uint32_t y, float val) {
			if(f(x, y, val))
			{ ret.push_back(PointType{static_cast<float>(x), static_cast<float>(y), val}); }
		});

		return ret;
	}
}

template<class T>
inline constexpr bool std::ranges::enable_borrowed_range<terraformer::span_2d<T>> = true;

#endif
