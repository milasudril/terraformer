#ifndef TERRAFORMER_LIB_SPAN2D_HPP
#define TERRAFORMER_LIB_SPAN2D_HPP

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
}

#endif