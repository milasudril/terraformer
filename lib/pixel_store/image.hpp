//@	{"targets":[{"name":"image.hpp", "type":"include"}]}

#ifndef TERRAFORMER_IMAGE_HPP
#define TERRAFORMER_IMAGE_HPP

#include "./rgba_pixel.hpp"

#include "lib/common/span_2d.hpp"

#include <algorithm>
#include <memory>
#include <vector>

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

		explicit basic_image(IndexType width, IndexType height)
		requires std::is_default_constructible_v<PixelType>:
		  m_width{width}
		, m_height{height}
		, m_data{std::make_unique<PixelType[]>(static_cast<size_t>(width)*static_cast<size_t>(height))}
		{}

		explicit basic_image(IndexType width, IndexType height, PixelType initial_value)
		requires (!std::is_default_constructible_v<PixelType>):
		  m_width{width}
		, m_height{height}
		, m_data{
			static_cast<PixelType*>(malloc(static_cast<size_t>(width)*static_cast<size_t>(height)*sizeof(PixelType)))
		}
		{
			std::uninitialized_fill_n(
				m_data.get(),
				static_cast<size_t>(width)*static_cast<size_t>(height),
				initial_value
			);
		}

		~basic_image() requires std::is_default_constructible_v<PixelType> = default;

		~basic_image() requires (!std::is_default_constructible_v<PixelType>)
		{ std::destroy_n(m_data.get(), static_cast<size_t>(m_width)*static_cast<size_t>(m_height)); }

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

		struct deleter
		{
			void operator()(PixelType* ptr)
			{ free(ptr); }
		};

		using holder = std::conditional_t<
			std::is_default_constructible_v<PixelType>,
			std::unique_ptr<PixelType[]>,
			std::unique_ptr<PixelType, deleter>
		>;

		holder m_data;

		PixelType const* getAddress(IndexType x, IndexType y) const
		{
			auto ptr = reinterpret_cast<PixelType const*>(m_data.get());
			return ptr + y * width() + x;
		}
	};

	template<class T, class PixelType>
	auto create_with_same_size(span_2d<PixelType> span)
	{	return basic_image<T>{span.width(), span.height()};	}

	template<class PixelType>
	auto create_with_same_size(span_2d<PixelType> span)
	{	return basic_image<PixelType>{span.width(), span.height()};	}

	using image = basic_image<rgba_pixel>;

	using grayscale_image = basic_image<float>;


	template<class T, class ValueSource, class DomainMask>
	requires (
		std::is_same_v<std::invoke_result_t<ValueSource, uint32_t, uint32_t>, T>
		&& std::is_same_v<std::invoke_result_t<DomainMask, uint32_t, uint32_t>, bool>
	)
	void floodfill(
		span_2d<T> output,
		pixel_coordinates start_at,
		ValueSource&& val_src,
		DomainMask&& is_inside_domain
	)
	{
		std::vector<pixel_coordinates> to_visit;
		to_visit.reserve(
			 static_cast<size_t>(output.width())
			*static_cast<size_t>(output.height())
		);

		auto visited = create_with_same_size<bool>(output);

		{
			auto const x = static_cast<uint32_t>(start_at.x);
			auto const y = static_cast<uint32_t>(start_at.y);
			if(inside(output, start_at.x, start_at.y) && is_inside_domain(x, y)) [[likely]]
			{ to_visit.push_back(start_at); }
		}

		while(!to_visit.empty())
		{
			auto item = to_visit.back();
			{
				to_visit.pop_back();
				auto const x = static_cast<uint32_t>(item.x);
				auto const y = static_cast<uint32_t>(item.y);
				output(x, y) = val_src(x, y);
			}

			// Search order optimized for scanlines
			std::array const neighbours{
				pixel_coordinates{
					item.x,
					item.y - 1
				},
				pixel_coordinates{
					item.x,
					item.y + 1
				},
				pixel_coordinates{
					item.x - 1,
					item.y
				},
				pixel_coordinates{
					item.x + 1,
					item.y
				}
			};

			for(auto neighbour: neighbours)
			{
				auto const x = static_cast<uint32_t>(neighbour.x);
				auto const y = static_cast<uint32_t>(neighbour.y);
				if(inside(output, neighbour.x, neighbour.y) && !visited(x, y) && is_inside_domain(x, y)) [[likely]]
				{
					visited(x, y) = true;
					to_visit.push_back(neighbour);
				}
			}
		}
	}
}

#endif
