#ifndef TERRAFORMER_MULTIRES_IMAGE_HPP
#define TERRAFORMER_MULTIRES_IMAGE_HPP

#include "lib/common/span_2d.hpp"

#include <array>

namespace terraformer
{
	template<class PixelType>
	class multires_image
	{
	public:
		static constexpr uint32_t scale_factor = 128;

		span_2d<PixelType const> pixels() const
		{
			return span_2d<PixelType const>{
				scale_factor,
				scale_factor,
				std::data(m_lowres_image)
			};
		}

		span_2d<PixelType> pixels()
		{
			return span_2d<PixelType >{
				scale_factor,
				scale_factor,
				std::data(m_lowres_image)
			};
		}

		span_2d<detail const> details() const
		{
			return span_2d<detail const>{
				scale_factor,
				scale_factor,
				std::data(m_highres_images)
			};
		}

		span_2d<detail> details()
		{
			return span_2d<detail>{
				scale_factor,
				scale_factor,
				std::data(m_highres_images)
			};
		}

	private:
		std::array<T, scale_factor*scale_factor> m_lowres_image{};
		using detail = std::unique_ptr<multires_image<T>;
		std::array<detail>, scale_factor*scale_factor> m_highres_images{};
	};
}

#endif