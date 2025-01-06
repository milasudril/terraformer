#ifndef TERRAFORMER_MULTIRES_IMAGE_HPP
#define TERRAFORMER_MULTIRES_IMAGE_HPP

#include "lib/common/span_2d.hpp"

namespace terraformer
{
	template<class PixelType>
	class multires_image
	{
	public:
		static constexpr uint32_t lowres_size = 128;

		span_2d<PixelType const> pixels() const
		{
			return span_2d<PixelType const>{
				lowres_size,
				lowres_size,
				std::data(m_lowres_image)
			};
		}

		span_2d<PixelType> pixels()
		{
			return span_2d<PixelType >{
				lowres_size,
				lowres_size,
				std::data(m_lowres_image)
			};
		}

		span_2d<detail const> details() const
		{
			return span_2d<detail const>{
				lowres_size,
				lowres_size,
				std::data(m_highres_images)
			};
		}

		span_2d<detail> details()
		{
			return span_2d<detail>{
				lowres_size,
				lowres_size,
				std::data(m_highres_images)
			};
		}

	private:
		std::array<T, lowres_size*lowres_size> m_lowres_image{};
		using detail = std::unique_ptr<multires_image<T>;
		std::array<detail>, lowres_size*lowres_size> m_highres_images{};
	}

	template<class PixelType>
	auto max_delta(multires_image<PixelType> const& img, uint32_t x, uint32_t y)
	{
		auto const pixels = img.pixels();
		auto const details = img.details();
		auto const w = pixels.width();
		auto const h = pixels.height();
		auto const value = pixels(x, y);
		auto const detail = details(x, y).get();
		if(detail == nullptr)
		{ return std::optinal<PixelType>{}; }

		auto const detal_pixels = detail->pixels();
		auto max = std::abs(value - detail_pixels(0u ,0u));

		for(uint32_t y = 0; y != h; ++y)
		{
			for(uint32_t x = 0; x != w; ++x)
			{ max = std::max(std::abs(value - detail_pixels(x ,y)), max); }
		}

		return std::optional{max};
	}
}

#endif