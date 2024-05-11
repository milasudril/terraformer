//@	{
//@		"dependencies":[{"ref":"harfbuzz", "origin":"pkg-config"}],
//@		"dependencies_extra":[{"ref":"./text_shaper.o", "rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_UI_FONT_HANDLING_TEXT_SHAPER_HPP
#define TERRAFORMER_UI_FONT_HANDLING_TEXT_SHAPER_HPP

#include "./glyph_renderer.hpp"

#include "lib/common/utils.hpp"

#include <hb.h>
#include <hb-ft.h>
#include <memory>
#include <string_view>

namespace terraformer::ui::font_handling
{
	struct hb_font_deleter
	{
		void operator()(hb_font_t* font)
		{ hb_font_destroy(font); }
	};

	using hb_font_handle = std::unique_ptr<hb_font_t, hb_font_deleter>;

	inline auto make_font(int size, glyph_renderer& renderer)
	{
		return hb_font_handle{hb_ft_font_create(renderer.set_font_size(size).get_face(),[](void*){})};
	}

	class font
	{
	public:
		explicit font(int font_size, glyph_renderer& renderer):
			m_handle{make_font(font_size, renderer)},
			m_renderer{renderer}
		{}

		auto get_hb_font() const
		{ return m_handle.get(); }

		auto& get_renderer() const
		{ return m_renderer; }

	private:
		hb_font_handle m_handle;
		std::reference_wrapper<glyph_renderer> m_renderer;
	};

	struct hb_buffer_deleter
	{
		void operator()(hb_buffer_t* buffer)
		{ hb_buffer_destroy(buffer); }
	};

	using hb_buffer_handle = std::unique_ptr<hb_buffer_t, hb_buffer_deleter>;

	struct shaping_result
	{
		std::reference_wrapper<glyph_renderer> renderer;
		size_t glyph_count;
		hb_glyph_info_t* glyph_info;
		hb_glyph_position_t* glyph_pos;
	};

	inline auto& get_glyph(shaping_result const& result, glyph_index index)
	{ return result.renderer.get().get_glyph(index); };

	class text_shaper
	{
	public:
		text_shaper():m_handle{hb_buffer_create()}{}

		text_shaper& append(std::basic_string_view<char8_t> buffer)
		{
			auto const buffer_length = narrowing_cast<int>(std::size(buffer));
			auto const text = reinterpret_cast<char const*>(std::data(buffer));

			if(m_clear_before_append)
			{
				hb_buffer_clear_contents(m_handle.get());
				m_clear_before_append = false;
			}

			hb_buffer_add_utf8(m_handle.get(), text, buffer_length, 0, buffer_length);
			return *this;
		}

		text_shaper& with(hb_direction_t direction)
		{
			hb_buffer_set_direction(m_handle.get(), direction);
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

		[[nodiscard]] auto run(font const& font)
		{
			hb_shape(font.get_hb_font(), m_handle.get(), nullptr, 0);
			unsigned int glyph_count{};
			auto const info = hb_buffer_get_glyph_infos(m_handle.get(), &glyph_count);
			m_clear_before_append = true;
			return shaping_result{
				.renderer = font.get_renderer(),
				.glyph_count = glyph_count,
				.glyph_info = info,
				.glyph_pos = hb_buffer_get_glyph_positions(m_handle.get(), &glyph_count)
			};
		}

	private:
		hb_buffer_handle m_handle;
		bool m_clear_before_append{false};
	};

	span_2d_extents compute_extents(shaping_result const& result);

	basic_image<uint8_t> render(shaping_result const& result);
}

#endif