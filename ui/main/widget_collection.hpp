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
		class iterator_invalidation_handler_ref
		{
		public:
			template<class Listener>
			explicit iterator_invalidation_handler_ref(std::reference_wrapper<Listener> listener):
				m_listener{&listener.get()},
				m_callback{
					[](void* handle, widget_collection const& source){
						static_cast<Listener*>(handle)->iterators_invalidated(source);
					}
				}
			{}

			void notify_listener(widget_collection const& source) const
			{ m_callback(m_listener, source); }

		private:
			void* m_listener;
			void (*m_callback)(void*, widget_collection const&);
		};

		using widget_array = compatible_multi_array_t<widget_collection_ref::widget_span>;

		using index_type = widget_array::index_type;

		static constexpr index_type npos{static_cast<size_t>(-1)};

		explicit widget_collection(iterator_invalidation_handler_ref iihr):
			m_iihr{iihr}
		{}

		template<widget Widget>
		widget_collection& append(
			std::reference_wrapper<Widget> w,
			widget_geometry const& initial_geometry,
			widget_state initial_state = make_default_widget_state<Widget>()
		)
		{
			m_objects.push_back(
				&w.get(),
				initial_state,
				box_size{},
				initial_geometry,
				widget_layer_stack{},
				[](void* obj, graphics_backend_ref backend) {
					return static_cast<Widget*>(obj)->prepare_for_presentation(backend);
				},
				[](void* obj, cursor_enter_event const& event, window_ref wr, ui_controller ui_ctrl) {
					static_cast<Widget*>(obj)->handle_event(event, wr, ui_ctrl);
				},
				[](void* obj, cursor_leave_event const& event, window_ref wr, ui_controller ui_ctrl) {
					static_cast<Widget*>(obj)->handle_event(event, wr, ui_ctrl);
				},
				[](void* obj, cursor_motion_event const& event, window_ref wr, ui_controller ui_ctrl) {
					static_cast<Widget*>(obj)->handle_event(event, wr, ui_ctrl);
				},
				[](void* obj, mouse_button_event const& mbe, window_ref wr, ui_controller ui_ctrl) {
					static_cast<Widget*>(obj)->handle_event(mbe, wr, ui_ctrl);
				},
				[](void* obj, keyboard_button_event const& kbe, window_ref wr, ui_controller ui_ctrl) {
					static_cast<Widget*>(obj)->handle_event(kbe, wr, ui_ctrl);
				},
				[](void* obj, typing_event te, window_ref wr, ui_controller ui_ctrl){
					static_cast<Widget*>(obj)->handle_event(te, wr, ui_ctrl);
				},
				[](void* obj, keyboard_focus_enter_event kfe, window_ref wr, ui_controller ui_ctrl) {
					static_cast<Widget*>(obj)->handle_event(kfe, wr, ui_ctrl);
				},
				[](void* obj, keyboard_focus_leave_event kle, window_ref wr, ui_controller ui_ctrl) {
					static_cast<Widget*>(obj)->handle_event(kle, wr, ui_ctrl);
				},
				[](void* obj, widget_width_request req) {
					return static_cast<Widget*>(obj)->compute_size(req);
				},
				[](void* obj, widget_height_request req) {
					return static_cast<Widget*>(obj)->compute_size(req);
				},
				[](void* obj, box_size size) {
					return static_cast<Widget*>(obj)->confirm_size(size);
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
				[](void* obj){
					return static_cast<Widget*>(obj)->get_layout();
				}
			);
			m_iihr.notify_listener(*this);
			return *this;
		}

		auto iterator_invalidation_handler() const
		{ return m_iihr; }

		widget_collection& append(widget_collection_ref::widget_span::reference const& val)
		{
			apply(
				[&objects = m_objects]<class... T>(T&&... vals){
					objects.push_back(std::forward<T>(vals)...);
				},
				val
			);
			m_iihr.notify_listener(*this);
			return *this;
		}

		auto get_attributes()
		{ return widget_collection_ref{m_objects.attributes()}; }

		auto get_attributes() const
		{ return widget_collection_view{m_objects.attributes()}; }

		auto element_indices()
		{ return m_objects.element_indices(); }

		void clear()
		{
			m_objects.clear();
			m_iihr.notify_listener(*this);
		}

	private:
		widget_array m_objects;
		iterator_invalidation_handler_ref m_iihr;
	};

	inline auto find_recursive(cursor_position pos, widget_collection& widgets)
	{ return find_recursive(pos, widgets.get_attributes());}

	inline void theme_updated(widget_collection const& widgets, config const& cfg)
	{ theme_updated(widgets.get_attributes(), cfg); }
}

#endif
