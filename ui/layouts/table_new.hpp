#ifndef TERRAFORMER_UI_LAYOUTS_TABLE_NEW_HPP
#define TERRAFORMER_UI_LAYOUTS_TABLE_NEW_HPP

#include "lib/common/spaces.hpp"

#include <span>

namespace terraformer::ui::layouts
{
	class table_new
	{
	public:
		enum class cell_array_tag:int{rows, columns};

		template<cell_array_tag order>
		struct cell_count
		{ size_t value; };

		using row_count = cell_count<cell_array_tag::rows>;
		using column_count = cell_count<cell_array_tag::columns>;

		template<cell_array_tag tag>
		class cell_size_array
		{
		public:
			cell_size_array() = default;

			explicit cell_size_array(cell_count<tag> size):
				m_values{std::make_unique<float[]>(size.value)},
				m_size{size}
			{}

		private:
			std::unique_ptr<float[]> m_values;
			cell_count<tag> m_size{};
		};

		enum class cell_order:size_t{row_major, column_major};

		explicit table(row_count num_rows):
			m_cell_order{cell_order::column_major},
			m_rows{num_rows}
		{}

		explicit table(column_count num_cols):
			m_cell_order{cell_order::row_major},
			m_cols{num_cols}
		{}

		/**
		 * Sets cell sizes to the absolute value given by the sizes
		 */
		void set_cell_sizes_to(std::span<scaling const> sizes_in);

		/**
		 * Adjusts cell widths given available_width
		 */
		void adjust_cell_widths(float available_width);

		/**
		 * Adjusts cell heights given available_height
		 */
		void adjust_cell_heights(float available_height);

		/**
		 * Fetches the current cell sizes, and returns the size of the layout
		 */
		scaling get_cell_sizes_into(std::span<scaling> sizes_out) const;

		/**
		 * Fetches all cell locations into locs_out
		 */
		void get_cell_locations_into(std::span<location> locs_out) const;

	private:
		cell_order m_cell_order;
		cell_size_array<cell_array_tag::rows> m_rows;
		cell_size_array<cell_array_tag::columns> m_cols;
	};
}

#endif
