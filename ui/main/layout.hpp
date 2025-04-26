#ifndef TERRAFORMER_UI_MAIN_LAYOUT_HPP
#define TERRAFORMER_UI_MAIN_LAYOUT_HPP

#include "ui/main/widget_geometry.hpp"

#include <span>
#include <utility>

namespace terraformer::ui::main
{
	/**
	 * \param obj the layout object
	 * \param sizes_in precalculated sizes
	 * \param available_width the width available for the layout
	 * \param available_height the height available for the layout
	 * \param sizes_out sizes computed by the layout
	 *
	 * # Layout run (Fixed widths)
	 *
	 * 1. Set cell sizes width zero width
	 *   * Compute a suitable widget height
	 *   * Set layout cell sizes to default
	 *
	 * 2. Set cell widths with available width
	 *   * Apply cell widths specified by layout
	 *   * Recompute widget height given new width
	 *   * Set layout cell heights to default
	 *
	 * 3. Set cell heights with available heights
	 *  * Apply cell heights specified by layout
	 *  * Recompute widget width given new height
	 *  * Set layout cell widths to default
	 */
	template<class T>
	concept layout(
		T& obj,
		std::span<scaling const> sizes_in,
		float available_width,
		float available_height.
		std::span<scaling> sizes_out,
		std::span<widget_geometry> locs_out
	)
	{
		/**
		 * Sets cell sizes to the absolute value given by the sizes
		 */
		{obj.set_cell_sizes_to(sizes_in)} -> std::same_as<void>;

		/**
		 * Adjusts cell widths given available_width
		 */
		{obj.adjust_cell_widths(available_width)} -> std::same_as<void>;

		/**
		 * Adjusts cell heights given available_height
		 */
		{obj.adjust_cell_heights(available_height)} -> std::same_as<void>;

		/**
		 * Fetches all cell locations into locs_out
		 */
		{std::as_const(obj).get_cell_locations_into(locs_out)} -> std::same_as<void>;

		/**
		 * Returns the space occupied by the layout
		 */
		{std::as_const(obj).get_dimensions()} -> std::same_as<scaling>;

	}
}

#endif
