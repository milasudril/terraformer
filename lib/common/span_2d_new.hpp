#ifndef TERRAFORMER_LIB_SPAN2D_NEW_HPP
#define TERRAFORMER_LIB_SPAN2D_NEW_HPP

#include <cstdint>
#include <type_traits>
#include <cstddef>
#include <cmath>

namespace terraformer
{
	template<class T, uint32_t TileSize>
	class span_2d_tiled
	{
	public:
		using IndexType = uint32_t;

		using mapped_type = T;

		static constexpr auto tile_size()
		{
			return TileSize;
		}

		constexpr explicit span_2d_tiled(): span_2d_tiled{0u, 0u, nullptr} {}

		constexpr explicit span_2d_tiled(IndexType w, IndexType h, T* ptr):
			m_tilecount_x{1 + (w - 1)/TileSize},
			m_tilecount_y{1 + (h - 1)/TileSize},
			m_ptr{ptr}
		{}

		constexpr auto tilecount_x() const { return m_tilecount_x; }

		constexpr auto tilecount_y() const { return m_tilecount_y; }

		constexpr T& operator()(IndexType x, IndexType y) const
		{
			auto const x_tile = x/TileSize;
			auto const y_tile = y/TileSize;
			auto const x_offset = x%TileSize;
			auto const y_offset = y%TileSize;
			auto const tile_start = m_ptr + y_tile*m_tilecount_x + x_tile;

			return *(tile_start + y_offset*TileSize + x_offset);
		}

	private:
		IndexType m_tilecount_x;
		IndexType m_tilecount_y;
		T* m_ptr;
	};

	template<uint32_t TileSize, class Func>
	void visit_tiles(uint32_t x_count, uint32_t y_count, Func&& f)
	{
		for(uint32_t k = 0; k != y_count; ++k)
		{
			for(uint32_t l = 0; l != x_count; ++l)
			{
				for(uint32_t y = 0; y != TileSize; ++y)
				{
					for(uint32_t x = 0; x != TileSize; ++x)
					{
						f(l*TileSize + x, k*TileSize + y);
					}
				}
			}
		}
	}
}

#endif
