#ifndef TERRAFORMER_UI_LAYOUTS_NONE_HPP
#define TERRAFORMER_UI_LAYOUTS_NONE_HPP

#include "./common_params.hpp"

#include "lib/common/spaces.hpp"
#include "lib/math_utils/ratio.hpp"
#include "lib/array_classes/span.hpp"
#include "lib/array_classes/single_array.hpp"

#include <span>
#include <variant>
#include <algorithm>

namespace terraformer::ui::layouts
{
	class none
	{
	public:
		enum class cell_size_mode{use_default, expand};

		explicit none(cell_size_mode size):
			m_cell_size_mode{size},
			m_cell_size{0.0f, 0.0f, 0.0f}
		{}

		void set_default_cell_sizes_to(span<box_size const> sizes_in)
		{
			m_cell_size = std::accumulate(
				std::begin(sizes_in),
				std::end(sizes_in),
				box_size{0.0f, 0.0f, 0.0f},
				[](box_size a, box_size b){
					return max(a, b);
				}
			);
		}

		void adjust_cell_widths(float available_width)
		{
			m_cell_size[0] = (m_cell_size_mode == cell_size_mode::use_default)?
				 m_cell_size[0]
				:available_width;
		}

		void adjust_cell_heights(float available_height)
		{
			m_cell_size[1] = (m_cell_size_mode == cell_size_mode::use_default)?
				m_cell_size[1]
				:available_height;
		}

		void get_cell_sizes_into(span<box_size> sizes_out) const
		{ std::ranges::fill(sizes_out, m_cell_size); }

		void get_cell_locations_into(span<location>) const
		{
			// TODO
		}

		box_size get_dimensions() const
		{ return m_cell_size; }

		auto& params()
		{ return m_params; }

	private:
		cell_size_mode m_cell_size_mode;
		common_params m_params{};
		box_size m_cell_size;
	};
}

#endif
