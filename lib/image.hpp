//@	{"targets":[{"name":"image.hpp", "type":"include"}]}

#ifndef TERRAFORMER_IMAGE_HPP
#define TERRAFORMER_IMAGE_HPP

#include "./rgba_pixel.hpp"

#include "./span_2d.hpp"

#include <algorithm>
#include <span>
#include <memory>

template<class PixelType>
class BasicImage
{
public:
	using IndexType = typename Span2d<PixelType>::IndexType;

	explicit BasicImage(IndexType width, IndexType height):
	  m_size{width, height}
	, m_data{std::make_unique<PixelType[]>(width*height)}
	{}

	BasicImage(BasicImage const& src): BasicImage{src.pixels()} {}

	explicit BasicImage(Span2d<PixelType const> src):BasicImage{src.width(), src.height()}
	{
		std::copy(std::begin(src), std::end(src), m_data.get());
	}

	BasicImage& operator=(BasicImage&&) = default;

	BasicImage(BasicImage&&) = default;

	BasicImage& operator=(BasicImage const&) = delete;

	auto width() const { return m_size.width(); }

	auto height() const { return m_size.depth(); }

	Extents<IndexType> extents() const { return m_size; }


	PixelType operator()(IndexType x, IndexType y) const { return *getAddress(x, y); }

	PixelType& operator()(IndexType x, IndexType y)
	{
		return *const_cast<PixelType*>(std::as_const(*this).getAddress(x, y));
	}

	Span2d<PixelType const> pixels() const
	{
		return Span2d<PixelType const>{width(), height(), reinterpret_cast<PixelType const*>(m_data.get())};
	}

	Span2d<PixelType> pixels()
	{
		return Span2d<PixelType>{width(), height(), reinterpret_cast<PixelType*>(m_data.get())};
	}

	operator Span2d<PixelType>() { return pixels(); }

	operator Span2d<PixelType const>() const { return pixels(); }

private:
	Extents<IndexType> m_size;
	std::unique_ptr<PixelType[]> m_data;

	PixelType const* getAddress(IndexType x, IndexType y) const
	{
		auto ptr = reinterpret_cast<PixelType const*>(m_data.get());
		return ptr + y * width() + x;
	}
};

template<class PixelType>
inline auto extents(BasicImage<PixelType> const& img)
{
	return img.extents();
}

template<class PixelType>
inline auto area(BasicImage<PixelType> const& img)
{
	return volume(img.extents());
}

using Image = BasicImage<RgbaPixel>;

using GrayscaleImage = BasicImage<float>;

#endif