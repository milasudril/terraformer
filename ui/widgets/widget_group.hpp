#ifndef TERRAFORMER_UI_WIDGETS_WIDGET_GROUP_HPP
#define TERRAFORMER_UI_WIDGETS_WIDGET_GROUP_HPP

#include "ui/main/widget_collection.hpp"

namespace terraformer::ui::widgets
{
	template<class Layout>
	class widget_group:public main::widget_with_default_actions, public main::widget_collection
	{
	public:
		widget_group(widget_group&&) = delete;
		widget_group& operator=(widget_group&&) = delete;
		widget_group(widget_group const&) = delete;
		widget_group& operator=(widget_group const&) = delete;

		using refresh_function = move_only_function<void()>;

		template<class ... LayoutArgs>
		explicit widget_group(iterator_invalidation_handler_ref iihr, LayoutArgs&&... args):
			widget_collection{iihr},
			layout{std::forward<LayoutArgs>(args)...},
			m_refresh_func{[](){}}
		{ }

		Layout layout;
		bool is_transparent = false;

		main::layout_ref get_layout()
		{ return main::layout_ref{layout}; }

		main::widget_collection_ref get_children()
		{ return get_attributes(); }

		main::widget_collection_view get_children() const
		{ return get_attributes(); }

		void theme_updated(main::config const& new_theme, main::widget_instance_info instance_info)
		{
			auto const& panel = instance_info.section_level%2 == 0?
				new_theme.main_panel :
				new_theme.other_panel;
			layout.params().margin_x = panel.padding;
			layout.params().margin_y = panel.padding;
			m_background = panel.background_texture;
			m_background_tint = panel.colors.background;
			m_null_texture = new_theme.misc_textures.null;
		}

 		main::widget_layer_stack prepare_for_presentation(main::graphics_backend_ref backend)
		{
			std::array const bg_tints{
				m_background_tint,
				m_background_tint,
				m_background_tint,
				m_background_tint
			};
			auto const null_texture = m_null_texture->get_backend_resource(backend).get();
			auto const background = m_background->get_backend_resource(backend).get();
			return main::widget_layer_stack{
				.background = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = is_transparent?null_texture:background,
					.tints = bg_tints
				},
				.sel_bg_mask = main::widget_layer_mask{
					.offset = displacement{},
					.texture = null_texture,
				},
				.selection_background = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				},
				.foreground = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				},
				.frame = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				},
				.input_marker = main::widget_layer{
					.offset = displacement{},
					.rotation = geosimd::turn_angle{},
					.texture = null_texture,
					.tints = std::array<rgba_pixel, 4>{}
				}
			};
		}

		template<class Callable>
		void set_refresh_function(Callable&& cb)
		{
			m_refresh_func = std::forward<Callable>(cb);
			m_refresh_func();
		}

		void refresh()
		{ m_refresh_func(); }

	private:
		main::immutable_shared_texture m_background;
		main::immutable_shared_texture m_null_texture;
		rgba_pixel m_background_tint;
		refresh_function m_refresh_func;
	};
}

#endif