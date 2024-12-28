#ifndef TERRAFORMER_UI_WIDGETS_RANGE_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_RANGE_INPUT_HPP

#include "./normalized_float_input.hpp"
#include "ui/main/widget.hpp"
#include "ui/main/graphics_backend_ref.hpp"

namespace terraformer::ui::widgets
{
	class range_input:public main::widget_with_default_actions
	{
	public:
		template<class OwnedType, class... Args>
		explicit range_input(std::in_place_type_t<OwnedType>, Args&&... args):
			m_view{std::in_place_type_t<OwnedType>{}, std::forward<Args>(args)...}
		{
			m_view_ref.append(
				std::ref(*static_cast<OwnedType*>(m_view.get().get_pointer())),
				main::widget_geometry{}
			);
			handle_event.attribs = m_view_ref.get_attributes();
		}

		template<class... Args>
		decltype(auto) prepare_for_presentation(Args&&... args)
		{
			auto const i = m_view_ref.element_indices().front();
			auto const attribs = m_view_ref.get_attributes();
			auto const ptr = attribs.widget_pointers()[i];
			auto const callback = attribs.render_callbacks()[i];
			return callback(ptr,std::forward<Args>(args)...);
		}

		template<class... Args>
		decltype(auto) compute_size(Args&&... args)
		{
			auto const i = m_view_ref.element_indices().front();
			auto const attribs = m_view_ref.get_attributes();
			auto const ptr = attribs.widget_pointers()[i];
			auto const callback = attribs.compute_size_callbacks<std::remove_cvref_t<Args>...>()[i];
			return callback(ptr, std::forward<Args>(args)...);
		}

		template<class... Args>
		decltype(auto) theme_updated(Args&&... args)
		{
			auto const i = m_view_ref.element_indices().front();
			auto const attribs = m_view_ref.get_attributes();
			auto const ptr = attribs.widget_pointers()[i];
			auto const callback = attribs.theme_updated_callbacks()[i];
			return callback(ptr, std::forward<Args>(args)...);
		}

	private:
		struct vtable
		{
			template<class T>
			constexpr vtable(T){};
		};
		unique_resource<vtable> m_view;
		main::widget_collection m_view_ref;

	public:
		struct handle_event_impl
		{
			template<class... Args>
			decltype(auto) operator()(Args&&... args)
			{
				auto const i = attribs.element_indices().front();
				auto const ptr = attribs.widget_pointers()[i];
				auto const callback = attribs.event_callbacks<std::remove_cvref_t<Args>...>()[i];
				return callback(ptr, std::forward<Args>(args)...);
			}

		private:
			friend class range_input;
			main::widget_collection_ref attribs;
		};

		handle_event_impl handle_event;
	};
}

#endif
