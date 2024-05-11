#ifndef TERRAFORMER_UI_FONT_HANDLING_TEXT_SHAPER_HPP
#define TERRAFORMER_UI_FONT_HANDLING_TEXT_SHAPER_HPP

#include "./glyph_renderer.hpp"

#include "lib/common/utils.hpp"

#include <hb.h>
#include <memory>

namespace terraformer::ui::font_handling
{
	struct hb_font_deleter
	{
		void operator()(hb_font_t* font)
		{ hb_font_destroy(font); }
	};

	using hb_font_handle = std::unique_ptr<hb_font_t>;

	auto make_hb_font(int size, glyph_renderer& renderer)
	{
		return hb_font_handle{hb_ft_font_create(renderer.set_font_size(size).get_face(),[](void*){})};
	}

	struct hb_buffer_deleter
	{
		void operator()(hb_buffer_t* buffer)
		{ hb_buffer_destroy(buffer); }
	};

	using hb_buffer_handle = std::unique_ptr<hb_buffer_t, hb_buffer_deleter>;

	struct shaping_result
	{
		size_t glyph_count;
		hb_glyph_info_t* glyph_info;
		hb_glyph_position_t* glyph_pos;
	};

	class text_shaper
	{
	public:
		text_shaper():m_handle{hb_buffer_create()}{}

		text_shaper& append(std::string_basic_view<char8_t> buffer)
		{
			auto const buffer_length = narrowing_cast<int>(std::size(buffer));
			if(m_clear_before_append)
			{
				hb_buffer_clear_contents(m_handle.get());
				m_clear_before_append = false;
			}
			hb_buffer_add_utf8(m_handle.get(), std::data(buffer), buffer_length, 0, buffer_length);
			return *this;
		}

		text_shaper& with(hb_direction_t direction)
		{
			hb_buffer_set_script(m_handle.get(), direction);
			return *this;
		}

		text_shaper& with(hb_script_t script)
		{
			hb_buffer_set_script(m_handle.get(), script);
			return *this;
		}

		text_shaper& with(hb_language_t lang)
		{
			hb_buffer_set_language(m_handle.get(), lang);
			return *this;
		}

		auto run(hb_font_t& font)
		{
			hb_shape(font, m_handle.get(), nullptr, 0);
			unsigned int glyph_count{};
			m_clear_before_append = true;
			return shaping_result{
				.glyph_count = glyph_count,
				.glyph_info = hb_buffer_get_glyph_infos(m_handle.get(), &glyph_count),
				.glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count)
			};
		}

	private:
		hb_buffer_handle m_handle;
		bool m_clear_before_append{false};
	};
}

#endif