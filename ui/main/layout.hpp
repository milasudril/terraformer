#ifndef TERRAFORMER_UI_MAIN_LAYOUT_HPP
#define TERRAFORMER_UI_MAIN_LAYOUT_HPP

#include "lib/common/spaces.hpp"

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
	concept layout = requires(
		T& obj,
		std::span<scaling const> sizes_in,
		float available_width,
		float available_height,
		std::span<scaling> sizes_out,
		std::span<location> locs_out
	)
	{
		/**
		 * Sets cell sizes to the absolute value given by the sizes
		 */
		{obj.set_default_cell_sizes_to(sizes_in)} -> std::same_as<void>;

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
		 * Fetches all cell locations into sizes_out
		 */
		{std::as_const(obj).get_cell_sizes_into(sizes_out)} -> std::same_as<void>;

		/**
		 * Returns the space occupied by the layout
		 */
		{std::as_const(obj).get_dimensions()} -> std::same_as<scaling>;
	};

	struct layout_vtable
	{
		void (*set_default_cell_sizes_to)(void*, std::span<scaling const>);
		void (*adjust_cell_widths)(void*, float);
		void (*adjust_cell_heights)(void*, float);
		void (*get_cell_sizes_into)(void const*, std::span<scaling>);
		void (*get_cell_locations_into)(void const*, std::span<location>);
		scaling (*get_dimensions)(void const*);
	};

	template<layout T>
	inline constexpr layout_vtable layout_vtable_v{
		.set_default_cell_sizes_to = [](void* obj, std::span<scaling const> vals) {
			static_cast<T*>(obj)->set_default_cell_sizes_to(vals);
		},
		.adjust_cell_widths = [](void* obj, float available_width){
			static_cast<T*>(obj)->adjust_cell_widths(available_width);
		},
		.adjust_cell_heights = [](void* obj, float available_height){
			static_cast<T*>(obj)->adjust_cell_heights(available_height);
		},
		.get_cell_sizes_into = [](void const* obj, std::span<scaling> sizes_out){
			static_cast<T const*>(obj)->get_cell_sizes_into(sizes_out);
		},
		.get_cell_locations_into = [](void const* obj, std::span<location> locs_out){
			static_cast<T const*>(obj)->get_cell_locations_into(locs_out);
		},
		.get_dimensions = [](void const* obj){
			return static_cast<T const*>(obj)->get_dimensions();
		}
	};

	class layout_ref
	{
	public:
		layout_ref() = default;

		template<layout T>
		explicit layout_ref(T& layout):
			m_vtable{&layout_vtable_v<T>},
			m_object{&layout}
		{}

		void set_default_cell_sizes_to(std::span<scaling const> vals) const
		{ m_vtable->set_default_cell_sizes_to(m_object, vals); }

		void adjust_cell_widths(float available_width) const
		{ m_vtable->adjust_cell_widths(m_object, available_width); }

		void adjust_cell_heights(float available_height) const
		{ m_vtable->adjust_cell_heights(m_object, available_height); }

		void get_cell_sizes_into(std::span<scaling> sizes_out) const
		{ m_vtable->get_cell_sizes_into(m_object, sizes_out); }

		void get_cell_locations_into(std::span<location> locs_out) const
		{ m_vtable->get_cell_locations_into(m_object, locs_out); }

		scaling get_dimensions() const
		{ return m_vtable->get_dimensions(m_object); }

		bool is_valid() const
		{ return m_vtable != nullptr; }

	private:
		layout_vtable const* m_vtable{nullptr};
		void* m_object{nullptr};
	};
}

#endif
