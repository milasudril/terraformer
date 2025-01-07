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
		std::array<PixelType, scale_factor*scale_factor> m_lowres_image{};
		using detail = std::unique_ptr<multires_image<PixelType>;
		std::array<detail>, scale_factor*scale_factor> m_highres_images{};
	};

	template<class PixelType, class Generator>
	std::unique_ptr<multires_image<PixelType>> generate(Generator const& g)
	{
		auto ret = std::make_unique<multires_image<PixelType>>();

		struct context
		{
			multires_image<PixelType>* tile;
			uint32_t origin_x;
			uint32_t origin_y;
			uint32_t scale;
		};

		single_array<context> contexts;
		contexts.reserve(3);

		contexts.push_back(
			context{
				.tile = ret.get(),
				.origin_x = 0u,
				.origin_y = 0u,
				.scale = 1u
			}
		);

		while(!contexts.empty())
		{
			auto current_context = std::move(contexts.back());
			contexts.pop_back();

			auto const tile_size = multires_image<PixelType>::scale_factor;
			auto const pixels = current_context.tile->pixels();
			auto const details = current_context.tile->details();

			// TODO: g needs origin_x, origin_y, and scale
			g(pixels);

			if(current_context.scale == tile_size*tile_size)
			{ continue; }

			auto const upscaled = std::make_unique<std::array<multires_image<PixelType>, 4*tile_size>();
			span_2d const upscaled_pixles{upscaled->data(), 2*tile_size, 2*tile_size};

			// TODO: g needs origin_x, origin_y, and scale
			g(upscaled_pixles);

			for(uint32_t y = 0; y != tile_size; ++y)
			{
				for(uint32_t x = 0; x != tile_size; x++)
				{
					auto const old_value = pixels(x, y);
					std::array const new_vals{
						upscaled_pixles(2*x, 2*y),
						upscaled_pixles(2*x, 2*y + 1),
						upscaled_pixles(2*x + 1, 2*y),
						upscaled_pixles(2*x + 1, 2*y + 1)
					};

					auto const max_delta = std::abs(
						*std::ranges::max_element(
							new_vals,
							[old_value](auto const& a, auto const& b){
								return std::abs(a - old_value) < std::abs(b - old_value);
							}
						) - old_value
					);

					if(max_delta > threshold)
					{
						details(x, y) = std::make_unique<multires_image<PixelType>>();
						context.push_back(
							context{
								.tile = detail(x, y).get(),
								.origin_x = x,
								.origin_y = y,
								.scale = current_context.scale*tile_size
							}
						)
					}
				}
			}
		}

		return ret;
	}
}

#endif