#ifndef TERRAFORMER_LIB_SPAN2D_HPP
#define TERRAFORMER_LIB_SPAN2D_HPP

#include "./utils.hpp"

#include <cstdint>
#include <type_traits>
#include <cstddef>

namespace terraformer
{
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

		constexpr T& operator()(IndexType x, IndexType y) const
		{
			auto ptr = begin();
			return *(ptr + y * width() + x);
		}

	private:
		IndexType m_width;
		IndexType m_height;
		T* m_ptr;
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

	template<class T, class U, class Func>
	void transform(span_2d<T> in, span_2d<U> out, Func&& f)
	{
		using IndexType = typename span_2d<T>::IndexType;
		for(IndexType row = 0; row != in.height(); ++row)
		{
			for(IndexType col = 0; col != in.width(); ++col)
			{
				out(col, row) = f(col, row, in(col, row));
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

	template<class T>
	T interp(span_2d<T const> img, float x, float y)
	{
		auto const w = img.width();
		auto const h = img.height();

		x = mod(x, static_cast<float>(w));
		y = mod(y, static_cast<float>(h));

		auto const x_0 = static_cast<uint32_t>(x);
		auto const y_0 = static_cast<uint32_t>(y);
		auto const x_1 = (x_0 + 1) % w;
		auto const y_1 = (y_0 + 1) % h;;

		auto const z_00 = img(x_0, y_0);
		auto const z_01 = img(x_0, y_1);
		auto const z_10 = img(x_1, y_0);
		auto const z_11 = img(x_1, y_1);

		auto const xi = x - static_cast<float>(x_0);
		auto const eta = y  - static_cast<float>(y_0);

		auto const z_x0 = (1.0f - static_cast<float>(xi)) * z_00 + static_cast<float>(xi) * z_10;
		auto const z_x1 = (1.0f - static_cast<float>(xi)) * z_01 + static_cast<float>(xi) * z_11;
		return (1.0f - eta)*z_x0 + eta*z_x1;
	}
}

#endif