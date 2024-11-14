//@	{"dependencies_extra":[{"ref":"./label.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_LABEL_HPP
#define TERRAFORMER_UI_WIDGETS_LABEL_HPP

#include "ui/main/generic_texture.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "lib/common/object_tree.hpp"

namespace terraformer::ui::widgets
{
	class label:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		template<class StringType>
		label& text(StringType&& str)
		{
			m_text = std::forward<StringType>(str);
			m_dirty_bits |= text_dirty | host_textures_dirty;
			return *this;
		}

		template<class StringType>
		label& value(StringType&& str)
		{ return text(std::forward<StringType>(str)); }

		std::basic_string_view<char8_t> value() const
		{ return m_text; }

		void regenerate_text_mask();

		void regenerate_textures();

		void prepare_for_presentation(main::widget_rendering_result output_rect);

		scaling compute_size(main::widget_width_request req);

		scaling compute_size(main::widget_height_request req);

		void handle_event(main::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= host_textures_dirty;
		}

		void theme_updated(const main::config& cfg, main::widget_instance_info);

	private:
		std::basic_string<char8_t> m_text;
		basic_image<uint8_t> m_rendered_text;
		static constexpr auto text_dirty = 0x1;
		static constexpr auto host_textures_dirty = 0x2;
		static constexpr auto gpu_textures_dirty = 0x4;
		unsigned int m_dirty_bits = text_dirty | host_textures_dirty | gpu_textures_dirty;
		float m_margin = 0;
		unsigned int m_border_thickness = 0;
		std::shared_ptr<font_handling::font const> m_font;
		rgba_pixel m_fg_tint;

		main::generic_unique_texture m_foreground;
		main::generic_shared_texture m_null_texture;

		main::fb_size m_current_size;
		image m_foreground_host;
	};
}

#endif
