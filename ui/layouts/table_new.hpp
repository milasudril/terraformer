//@	{"dependencies_extra":[{"ref":"./table_new.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_LAYOUTS_TABLE_NEW_HPP
#define TERRAFORMER_UI_LAYOUTS_TABLE_NEW_HPP

#include "./common_params.hpp"

#include "lib/common/spaces.hpp"
#include "lib/math_utils/ratio.hpp"
#include "lib/array_classes/span.hpp"

#include <span>
#include <memory>
#include <variant>
#include <algorithm>

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

			explicit cell_size_array(span<Value const> vals):
				m_values{std::make_unique_for_overwrite<Value[]>(std::size(vals).get())},
				m_size{cell_count<tag>{std::size(vals).get()}}
			{ std::copy(std::begin(vals), std::end(vals), std::begin(*this)); }

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

			Value* begin()
			{ return m_values.get(); }

			Value* end()
			{ return m_values.get() + std::size(*this); }

			operator span<Value>()
			{ return span<Value>{std::begin(*this), std::end(*this)}; }

			operator span<Value const>() const
			{ return span<Value const>{std::cbegin(*this), std::cend(*this)}; }

		private:
			std::unique_ptr<Value[]> m_values;
			cell_count<tag> m_size{};
		};

		template<class Value>
		using row_array = cell_size_array<Value, cell_array_tag::rows>;

		template<class Value>
		using column_array = cell_size_array<Value, cell_array_tag::columns>;

		struct cell_size
		{
			struct use_default{};
			struct expand{};
			struct fixed
			{ float value; };

			std::variant<use_default, expand, ratio, fixed> value;
		};

		static row_array<cell_size> rows(span<cell_size const> sizes)
		{ return row_array<cell_size>{sizes}; }

		static row_array<cell_size> rows(std::initializer_list<cell_size> const& sizes)
		{ return row_array<cell_size>{span{std::begin(sizes), std::end(sizes)}}; }

		template<class Head, class ... Tail>
		static row_array<cell_size> rows(Head&& h, Tail&&... t)
		{
			return rows({
				cell_size{.value = std::forward<Head>(h)},
				cell_size{.value = std::forward<Tail>(t)}...
			});
		}

		static column_array<cell_size> columns(span<cell_size const> sizes)
		{ return column_array<cell_size>{sizes}; }

		static column_array<cell_size> columns(std::initializer_list<cell_size> const& sizes)
		{ return column_array<cell_size>{span{std::begin(sizes), std::end(sizes)}}; }

		template<class Head, class ... Tail>
		static column_array<cell_size> columns(Head&& h, Tail&&... t)
		{
			return columns({
				cell_size{.value = std::forward<Head>(h)},
				cell_size{.value = std::forward<Tail>(t)}...
			});
		}


		enum class cell_order:size_t{row_major, column_major};

		explicit table_new(size_t fixdim_size, cell_order cell_order):
			m_cell_order{cell_order}
		{
			switch(m_cell_order)
			{
				case cell_order::row_major:
					m_cols = column_array<float>{column_count{fixdim_size}};
					m_cols_user = column_array<cell_size>{column_count{fixdim_size}};
					break;
				case cell_order::column_major:
					m_rows = row_array<float>{row_count{fixdim_size}};
					m_rows_user = row_array<cell_size>{row_count{fixdim_size}};
					break;
			}
		}

		explicit table_new(row_count num_rows):
			m_cell_order{cell_order::column_major},
			m_rows{num_rows},
			m_rows_user{num_rows}
		{}

		explicit table_new(row_array<cell_size>&& rows):
			m_cell_order{cell_order::column_major},
			m_rows{row_count{std::size(rows)}},
			m_rows_user{std::move(rows)}
		{}

		explicit table_new(column_count num_cols):
			m_cell_order{cell_order::row_major},
			m_cols{num_cols},
			m_cols_user{num_cols}
		{}

		explicit table_new(column_array<cell_size>&& cols):
			m_cell_order{cell_order::row_major},
			m_cols{column_count{std::size(cols)}},
			m_cols_user{std::move(cols)}
		{}

		/**
		 * Sets cell sizes to the absolute value given by the sizes
		 */
		void set_default_cell_sizes_to(span<scaling const> sizes_in);
		static row_array<float> set_default_cell_sizes_to(
			span<scaling const> sizes_in,
			column_array<float>& col_widths
		);
		static column_array<float> set_default_cell_sizes_to(
			span<scaling const> sizes_in,
			row_array<float>& row_heights
		);

		/**
		 * Adjusts cell widths given available_width
		 */
		void adjust_cell_widths(float available_width)
		{ adjust_cell_sizes(m_cols_user, m_cols, available_width, m_params.margin_x, m_params.no_outer_margin); }

		/**
		 * Adjusts cell heights given available_height
		 */
		void adjust_cell_heights(float available_height)
		{ adjust_cell_sizes(m_rows_user, m_rows, available_height, m_params.margin_y, m_params.no_outer_margin); }

		static float adjust_cell_sizes(
			span<cell_size const> specified_sizes,
			span<float> actual_sizes,
			float available_size,
			float margin,
			bool no_outer_margin
		);

		/**
		 * Fetches the current cell sizes
		 */
		void get_cell_sizes_into(span<scaling> sizes_out) const;
		static void get_cell_sizes_into(
			span<scaling> sizes_out,
			row_array<float> const& row_heights,
			column_array<float> const& col_widths
		);
		static void get_cell_sizes_into(
			span<scaling> sizes_out,
			column_array<float> const& col_widths,
			row_array<float> const& row_heights
		);

		/**
		 * Fetches all cell locations into locs_out
		 */
		void get_cell_locations_into(span<location> locs_out) const;
		static void get_cell_locations_into(
			span<location> locs_out,
			row_array<float> const& row_heights,
			column_array<float> const& col_widths,
			common_params const& params
		);
		static void get_cell_locations_into(
			span<location> locs_out,
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
		row_array<cell_size> m_rows_user;
		column_array<cell_size> m_cols_user;
		common_params m_params{};
	};
}

#endif
