//@	{"dependencies_extra":[{"ref":"./table_new.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_TABLE_NEW_HPP
#define TERRAFORMER_UI_LAYOUTS_TABLE_NEW_HPP

#include "./common_params.hpp"

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

		template<class Value, cell_array_tag tag>
		class cell_size_array
		{
		public:
			cell_size_array() = default;

			explicit cell_size_array(cell_count<tag> size):
				m_values{std::make_unique<Value[]>(size.value)},
				m_size{size}
			{}

			size_t size() const
			{ return m_size.value; }

			Value& operator[](size_t index)
			{ return m_values.get()[index]; }

			Value operator[](size_t index) const
			{ return m_values.get()[index]; }

			Value const* begin() const
			{ return m_values.get(); }

			Value const* end() const
			{ return m_values.get() + std::size(*this); }

		private:
			std::unique_ptr<Value[]> m_values;
			cell_count<tag> m_size{};
		};

		template<class Value>
		using row_array = cell_size_array<Value, cell_array_tag::rows>;

		template<class Value>
		using column_array = cell_size_array<Value, cell_array_tag::columns>;

		enum class cell_order:size_t{row_major, column_major};

		explicit table_new(size_t fixdim_size, cell_order cell_order):
			m_cell_order{cell_order}
		{
			switch(m_cell_order)
			{
				case cell_order::row_major:
					m_cols = column_array<float>{column_count{fixdim_size}};
					break;
				case cell_order::column_major:
					m_rows = row_array<float>{row_count{fixdim_size}};
					break;
			}
		}

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
		void set_default_cell_sizes_to(std::span<scaling const> sizes_in);
		static row_array<float> set_default_cell_sizes_to(
			std::span<scaling const> sizes_in,
			column_array<float>& col_widths
		);
		static column_array<float> set_default_cell_sizes_to(
			std::span<scaling const> sizes_in,
			row_array<float>& row_heights
		);

		/**
		 * Adjusts cell widths given available_width
		 */
		void adjust_cell_widths([[maybe_unused]] float available_width)
		{
			//TODO
		}

		/**
		 * Adjusts cell heights given available_height
		 */
		void adjust_cell_heights([[maybe_unused]] float available_height)
		{
			//TODO
		}

		/**
		 * Fetches the current cell sizes
		 */
		void get_cell_sizes_into(std::span<scaling> sizes_out) const;
		static void get_cell_sizes_into(
			std::span<scaling> sizes_out,
			row_array<float> const& row_heights,
			column_array<float> const& col_widths
		);
		static void get_cell_sizes_into(
			std::span<scaling> sizes_out,
			column_array<float> const& col_widths,
			row_array<float> const& row_heights
		);

		/**
		 * Fetches all cell locations into locs_out
		 */
		void get_cell_locations_into(std::span<location> locs_out) const;
		static void get_cell_locations_into(
			std::span<location> locs_out,
			row_array<float> const& row_heights,
			column_array<float> const& col_widths,
			common_params const& params
		);
		static void get_cell_locations_into(
			std::span<location> locs_out,
			column_array<float> const& col_widths,
			row_array<float> const& row_heights,
			common_params const& params
		);

		scaling get_dimensions() const;

		auto& params()
		{ return m_params; }

	private:
		cell_order m_cell_order;
		row_array<float> m_rows;
		column_array<float> m_cols;
		common_params m_params{};
	};
}

#endif
