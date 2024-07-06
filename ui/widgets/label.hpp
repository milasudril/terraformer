//@	{"dependencies_extra":[{"ref":"./label.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_LABEL_HPP
#define TERRAFORMER_UI_WIDGETS_LABEL_HPP

#include "./generic_texture.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "ui/font_handling/text_shaper.hpp"
#include "ui/main/widget.hpp"
#include "lib/common/object_tree.hpp"

namespace terraformer::ui::widgets
{
	class label
	{
	public:
		enum class state{released, pressed};

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

		template<class OutputRectangle>
		void prepare_for_presentation(
			OutputRectangle& output_rect,
			main::widget_instance_info const&,
			object_dict const& render_resources
		);

		void handle_event(wsapi::cursor_enter_leave_event const&)
		{ }

		void handle_event(wsapi::cursor_motion_event const&, main::input_device_grab&)
		{ }

		void handle_event(wsapi::mouse_button_event const&, main::input_device_grab&)
		{ }

		main::widget_size_constraints get_size_constraints() const;

		void handle_event(wsapi::fb_size size)
		{
			m_current_size = size;
			m_dirty_bits |= host_textures_dirty;
		}

		void theme_updated(object_dict const& render_resources);

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

		generic_unique_texture m_foreground;

		wsapi::fb_size m_current_size;
		image m_foreground_host;
	};

	template<class OutputRectangle>
	void label::prepare_for_presentation(
		OutputRectangle& output_rect,
		main::widget_instance_info const&,
		object_dict const& render_resources
	)
	{
		if(m_dirty_bits & host_textures_dirty) [[unlikely]]
		{ regenerate_textures(); }

		output_rect.foreground = m_foreground.get_const();
		if(!output_rect.foreground)
		{
			m_foreground = output_rect.create_texture();
			output_rect.foreground = m_foreground.get_const();
			m_dirty_bits |= gpu_textures_dirty;
		}

		output_rect.background = render_resources/"ui"/"null_texture";
		if(m_dirty_bits & gpu_textures_dirty)
		{
			m_foreground.upload(std::as_const(m_foreground_host).pixels());
			m_dirty_bits &= ~gpu_textures_dirty;
		}

		auto const fg_tint = (render_resources/"ui"/"output_area"/"text_color").get_if<rgba_pixel const>();
		assert(fg_tint != nullptr);
		output_rect.foreground_tints = std::array{*fg_tint, *fg_tint, *fg_tint, *fg_tint};
	}
}

#endif
