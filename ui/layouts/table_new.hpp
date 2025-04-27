//@	{"dependencies_extra":[{"ref":"./table_new.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_TABLE_NEW_HPP
#define TERRAFORMER_UI_LAYOUTS_TABLE_NEW_HPP

#include "lib/common/spaces.hpp"

#include <span>
#include <memory>

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

			size_t size() const
			{ return m_size.value; }

			float& operator[](size_t index)
			{ return m_values.get()[index]; }

			float operator[](size_t index) const
			{ return m_values.get()[index]; }

			float const* begin() const
			{ return m_values.get(); }

			float const* end() const
			{ return m_values.get() + std::size(*this); }

		private:
			std::unique_ptr<float[]> m_values;
			cell_count<tag> m_size{};
		};

		using row_array = cell_size_array<cell_array_tag::rows>;
		using column_array = cell_size_array<cell_array_tag::columns>;

		enum class cell_order:size_t{row_major, column_major};

		explicit table_new(row_count num_rows):
			m_cell_order{cell_order::column_major},
			m_rows{num_rows}
		{}

		explicit table_new(column_count num_cols):
			m_cell_order{cell_order::row_major},
			m_cols{num_cols}
		{}

		/**
		 * Sets cell sizes to the absolute value given by the sizes
		 */
		void set_cell_sizes_to(std::span<scaling const> sizes_in);
		static row_array set_cell_sizes_to(std::span<scaling const> sizes_in, column_array& col_widths);
		static column_array set_cell_sizes_to(std::span<scaling const> sizes_in, row_array& row_heights);

		/**
		 * Adjusts cell widths given available_width
		 */
		void adjust_cell_widths(float available_width);

		/**
		 * Adjusts cell heights given available_height
		 */
		void adjust_cell_heights(float available_height);

		/**
		 * Fetches the current cell sizes
		 */
		void get_cell_sizes_into(std::span<scaling> sizes_out) const;
		static void get_cell_sizes_into(
			std::span<scaling> sizes_out,
			row_array const& row_heights,
			column_array const& col_widths
		);
		static void get_cell_sizes_into(
			std::span<scaling> sizes_out,
			column_array const& col_widths,
			row_array const& row_heights
		);

		/**
		 * Fetches all cell locations into locs_out
		 */
		void get_cell_locations_into(std::span<location> locs_out) const;
		static void get_cell_locations_into(
			std::span<location> locs_out,
			row_array const& row_heights,
			column_array const& col_widths
		);
		static void get_cell_locations_into(
			std::span<location> locs_out,
			column_array const& col_widths,
			row_array const& row_heights
		);

		scaling get_dimensions() const;

	private:
		cell_order m_cell_order;
		row_array m_rows;
		column_array m_cols;
	};
}

#endif
