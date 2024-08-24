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
	class label
	{
	public:
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

		void handle_event(main::cursor_enter_leave_event const&)
		{ }

		void handle_event(main::cursor_motion_event const&)
		{ }

		void handle_event(main::mouse_button_event const&)
		{ }

		main::widget_size_constraints update_geometry();

		void handle_event(main::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= host_textures_dirty;
		}

		void theme_updated(object_dict const& render_resources);

		main::widget_collection_view get_children() const
		{ return main::widget_collection_view{}; }

	private:
		std::basic_string<char8_t> m_text;
		basic_image<uint8_t> m_rendered_text;
		static constexpr auto text_dirty = 0x1;
		static constexpr auto host_textures_dirty = 0x2;
		static constexpr auto gpu_textures_dirty = 0x4;
		unsigned int m_dirty_bits = text_dirty | host_textures_dirty | gpu_textures_dirty;
		unsigned int m_margin = 0;
		unsigned int m_border_thickness = 0;
		shared_const_any m_font;
		shared_const_any m_background;
		rgba_pixel m_fg_tint;

		main::generic_unique_texture m_foreground;

		main::fb_size m_current_size;
		image m_foreground_host;
	};

	inline void label::prepare_for_presentation(main::widget_rendering_result output_rect)
	{
		if(m_dirty_bits & host_textures_dirty) [[unlikely]]
		{ regenerate_textures(); }

		if(output_rect.set_foreground(m_foreground.get()) != main::set_texture_result::success) [[unlikely]]
		{
			m_foreground = output_rect.create_texture();
			output_rect.set_foreground(m_foreground.get());
			m_dirty_bits |= gpu_textures_dirty;
		}

		(void)output_rect.set_background(m_background.get_if<main::generic_unique_texture const>()->get());

		if(m_dirty_bits & gpu_textures_dirty)
		{
			m_foreground.upload(std::as_const(m_foreground_host).pixels());
			m_dirty_bits &= ~gpu_textures_dirty;
		}

		output_rect.set_foreground_tints(std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint});
	}
}

#endif
