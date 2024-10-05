//@	{"dependencies_extra":[{"ref":"./widget_collection.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_MAIN_WIDGET_COLLECTION_HPP
#define TERRAFORMER_UI_MAIN_WIDGET_COLLECTION_HPP

#include "./widget.hpp"
#include "lib/array_classes/multi_array.hpp"
#include "lib/pixel_store/rgba_pixel.hpp"

namespace terraformer::ui::main
{
	class widget_collection
	{
	public:
		using widget_array = compatible_multi_array_t<widget_collection_ref::widget_span>;

		using index_type = widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		template<class Widget>
		requires widget<Widget>
		widget_collection& append(
			std::reference_wrapper<Widget> w,
			widget_geometry const& initial_geometry,
			widget_state initial_state = make_default_widget_state<Widget>()
		)
		{
			m_objects.push_back(
				&w.get(),
				initial_state,
				scaling{},
				initial_geometry,
				[](void* obj, widget_rendering_result result) {
					return static_cast<Widget*>(obj)->prepare_for_presentation(result);
				},
				[](void* obj, cursor_enter_event const& event) {
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, cursor_leave_event const& event) {
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, cursor_motion_event const& event) {
					static_cast<Widget*>(obj)->handle_event(event);
				},
				[](void* obj, mouse_button_event const& mbe) {
					static_cast<Widget*>(obj)->handle_event(mbe);
				},
				[](void* obj, widget_width_request req) -> scaling{
					return static_cast<Widget*>(obj)->compute_size(req);
				},
				[](void* obj, widget_height_request req) -> scaling{
					return static_cast<Widget*>(obj)->compute_size(req);
				},
				[](void* obj, fb_size size) {
					static_cast<Widget*>(obj)->handle_event(size);
				},
				[](void* obj, config const& new_theme, widget_instance_info instance_info) {
					static_cast<Widget*>(obj)->theme_updated(new_theme, instance_info);
				},
				[](void* obj){
					return static_cast<Widget*>(obj)->get_children();
				},
				[](void const* obj){
					return static_cast<Widget const*>(obj)->get_children();
				},
				[](void const* obj){
					return static_cast<Widget const*>(obj)->get_layout();
				}
			);

			return *this;
		}

		widget_collection& append(widget_collection_ref::widget_span::reference const& val)
		{
			apply(
				[&objects = m_objects]<class... T>(T&&... vals){
					objects.push_back(std::forward<T>(vals)...);
				},
				val
			);
			return *this;
		}

		auto get_attributes()
		{ return widget_collection_ref{m_objects.attributes()}; }

		auto get_attributes() const
		{ return widget_collection_view{m_objects.attributes()}; }

		constexpr auto first_element_index()
		{ return m_objects.first_element_index(); }

	private:
		widget_array m_objects;
	};

	inline auto find(cursor_position pos, widget_collection const& widgets)
	{ return find(pos, widgets.get_attributes());}

	inline void theme_updated(widget_collection const& widgets, config const& cfg)
	{ theme_updated(widgets.get_attributes(), cfg); }

	inline auto find(find_recursive_result const& res, widget_collection const& widgets)
	{ return find(res, widgets.get_attributes()); }

	widget_collection flatten(widget_collection_ref const& widgets);
}

#endif
