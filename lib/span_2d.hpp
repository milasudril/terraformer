#ifndef TERRAFORMER_SPAN2D_HPP
#define TERRAFORMER_SPAN2D_HPP

#include "./extents.hpp"

#include <cstdint>

template<class T>
class Span2d
{
public:
	using IndexType = uint32_t;

	constexpr explicit Span2d(): Span2d{nullptr, Extents{0, 0}} {}

	constexpr explicit Span2d(T* ptr, IndexType w, IndexType h): Span2d{ptr, Extents{w, h}} {}

	template<class U>
	constexpr explicit(!std::is_same_v<std::decay_t<U>, std::decay_t<T>>)
		Span2d(Span2d<U> other)
		: m_ptr{other.data()}
		, m_size{other.size()}
	{
	}

	constexpr auto begin() const { return m_ptr; }

	constexpr auto begin() { return m_ptr; }

	constexpr auto end() const { return m_ptr + volume(m_size); }

	constexpr auto end() { return m_ptr + volume(m_size); }

	constexpr auto data() const { return begin(); }

	constexpr auto data() { return begin(); }

	constexpr auto width() const { return m_size.width(); }

	constexpr auto height() const { return m_size.height(); }

	constexpr T& operator()(IndexType x, IndexType y)
	{
		auto ptr = begin();
		return *(ptr + y * width() + x);
	}

	constexpr T const& operator()(IndexType x, IndexType y) const
	{
		auto ptr = begin();
		return *(ptr + y * width() + x);
	}

	constexpr auto extents() const { return m_size; }

private:
	T* m_ptr;
	Extents<IndexType> m_size;
};

template<class T>
constexpr inline auto area(Span2d<T> s)
{
	return volume(s.extents());
}

template<class T>
constexpr inline auto aspectRatio(Span2d<T> s)
{
	return aspectRatio(s.size());
}

template<class T, class Func>
void for_each(Span2d<T> span, Func&& f)
{
	using IndexType = typename Span2d<T>::IndexType;
	for(IndexType row = 0; row != span.height(); ++row)
	{
		for(IndexType col = 0; col != span.width(); ++col)
		{
			f(col, row, span(col, row));
		}
	}
}

template<class T, class U, class Func>
void transform(Span2d<T> in, Span2d<U> out, Func&& f)
{
	using IndexType = typename Span2d<T>::IndexType;
	for(IndexType row = 0; row != in.height(); ++row)
	{
		for(IndexType col = 0; col != in.width(); ++col)
		{
			out(col, row) = f(col, row, in(col, row));
		}
	}
}

template<class T, class Func>
void generate(Span2d<T> out, Func&& f)
{
	using IndexType = typename Span2d<T>::IndexType;
	for(IndexType row = 0; row != out.height(); ++row)
	{
		for(IndexType col = 0; col != out.width(); ++col)
		{
			out(col, row) = f(col, row);
		}
	}
}

#endif