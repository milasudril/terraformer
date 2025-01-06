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

		span_2d<PixelType const> details() const
		{
			return span_2d<PixelType const>{
				lowres_size,
				lowres_size,
				std::data(m_highres_images)
			};
		}

		span_2d<PixelType> details()
		{
			return span_2d<PixelType >{
				lowres_size,
				lowres_size,
				std::data(m_highres_images)
			};
		}

	private:
		std::array<T, lowres_size*lowres_size> m_lowres_image{};
		std::array<std::unique_ptr<multires_image<T>>, lowres_size*lowres_size> m_highres_images{};
	}
}

#endif