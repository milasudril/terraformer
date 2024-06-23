//@	{"targets":[{"name":"image.hpp", "type":"include"}]}

#ifndef TERRAFORMER_IMAGE_HPP
#define TERRAFORMER_IMAGE_HPP

#include "./rgba_pixel.hpp"

#include "lib/common/span_2d.hpp"

#include <algorithm>
#include <memory>

namespace terraformer
{
	template<class PixelType>
	class basic_image
	{
	public:
		using mapped_type = PixelType;

		using IndexType = typename span_2d<PixelType>::IndexType;

		basic_image() = default;

		explicit basic_image(span_2d_extents size):basic_image{size.width, size.height}{}

		explicit basic_image(IndexType width, IndexType height):
		  m_width{width}
		, m_height{height}
		, m_data{std::make_unique<PixelType[]>(static_cast<size_t>(width)*static_cast<size_t>(height))}
		{}

		basic_image(basic_image const& src): basic_image{src.pixels()} {}

		explicit basic_image(span_2d<PixelType const> src):basic_image{src.width(), src.height()}
		{
			std::copy(std::begin(src), std::end(src), m_data.get());
		}

		basic_image& operator=(basic_image&&) = default;

		basic_image(basic_image&&) = default;

		basic_image& operator=(basic_image const&) = delete;

		auto width() const { return m_width; }

		auto height() const { return m_height; }

		auto& width_ref() const { return m_width; }

		auto& height_ref() const { return m_height; }

		PixelType operator()(IndexType x, IndexType y) const { return *getAddress(x, y); }

		PixelType& operator()(IndexType x, IndexType y)
		{
			return *const_cast<PixelType*>(std::as_const(*this).getAddress(x, y));
		}

		span_2d<PixelType const> pixels() const
		{
			return span_2d<PixelType const>{width(), height(), reinterpret_cast<PixelType const*>(m_data.get())};
		}

		span_2d<PixelType> pixels()
		{
			return span_2d<PixelType>{width(), height(), reinterpret_cast<PixelType*>(m_data.get())};
		}

		operator span_2d<PixelType>() { return pixels(); }

		operator span_2d<PixelType const>() const { return pixels(); }

		bool has_pixels() const { return m_data != nullptr; }

	private:
		IndexType m_width{0};
		IndexType m_height{0};
		std::unique_ptr<PixelType[]> m_data;

		PixelType const* getAddress(IndexType x, IndexType y) const
		{
			auto ptr = reinterpret_cast<PixelType const*>(m_data.get());
			return ptr + y * width() + x;
		}
	};

	template<class T, class PixelType>
	auto create_with_same_size(span_2d<PixelType> span)
	{	return basic_image<T>{span.width(), span.height()};	}

	using image = basic_image<rgba_pixel>;

	using grayscale_image = basic_image<float>;
}

#endif
