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

	inline auto make_font(glyph_renderer& renderer)
	{
		return hb_font_handle{hb_ft_font_create(renderer.get_face(), [](void*){})};
	}

	class font
	{
	public:
		explicit font(std::filesystem::path const& filename):
			m_renderer{std::make_unique<glyph_renderer>(filename)},
			m_handle{make_font(*m_renderer)}
		{}

		auto get_hb_font() const
		{ return m_handle.get(); }

		auto& get_renderer() const
		{ return *m_renderer; }

		auto& set_font_size(int new_size)
		{
			m_renderer->set_font_size(new_size);
			hb_ft_font_changed(m_handle.get());
			return *this;
		}

	private:
		std::unique_ptr<glyph_renderer> m_renderer;
		hb_font_handle m_handle;
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

	span_2d_extents compute_extents(shaping_result const& result);

	basic_image<uint8_t> render(shaping_result const& result);

	inline auto& get_glyph(shaping_result const& result, glyph_index index)
	{ return result.renderer.get().get_glyph(index); };

	struct glyph_sequence
	{
	public:
		glyph_sequence() = default;

		using storage_type = multi_array<location, size_t, glyph const*>;
		using size_type = storage_type::size_type;
		using index_type = storage_type::index_type;

		explicit glyph_sequence(shaping_result const&);

		auto element_indices() const
		{ return m_content.element_indices(); }

		auto locations() const
		{ return m_content.get<0>(); }

		auto input_indices() const
		{ return m_content.get<1>(); }

		auto glyph_pointers() const
		{ return m_content.get<2>(); }

		auto extents() const
		{ return m_extents; }

		auto glyph_count() const
		{ return std::size(m_content).get(); }

	private:
		storage_type m_content;
		span_2d_extents m_extents;
	};

	terraformer::basic_image<uint8_t> render(glyph_sequence const& seq);

	struct glyph_geometry
	{
		location loc;
		displacement advance;
	};

	inline auto input_index_to_location(glyph_sequence const& seq, size_t index)
	{
		auto const find_iter = std::ranges::find(seq.input_indices(), index);
		if(find_iter == std::end(seq.input_indices()))
		{ return std::optional<glyph_geometry>{}; }

		glyph_sequence::index_type const i{static_cast<size_t>(find_iter - std::begin(seq.input_indices()))};
		auto const& glyph = *seq.glyph_pointers()[i];

		return std::optional{
			glyph_geometry{
				.loc = seq.locations()[i],
				.advance = displacement{
					static_cast<float>(glyph.image.width()),
					static_cast<float>(glyph.image.height()),
					0.0f
				}
			}
		};
	}


	class text_shaper
	{
	public:
		text_shaper():m_handle{hb_buffer_create()}
		{ hb_buffer_set_cluster_level(m_handle.get(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS); }

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

		text_shaper& append(std::basic_string_view<char32_t> buffer)
		{
			auto const buffer_length = narrowing_cast<int>(std::size(buffer));
			auto const text = reinterpret_cast<uint32_t const*>(std::data(buffer));

			if(m_clear_before_append)
			{
				hb_buffer_clear_contents(m_handle.get());
				m_clear_before_append = false;
			}

			hb_buffer_add_utf32(m_handle.get(), text, buffer_length, 0, buffer_length);
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
			return glyph_sequence{
				shaping_result{
					.renderer = font.get_renderer(),
					.glyph_count = glyph_count,
					.glyph_info = info,
					.glyph_pos = hb_buffer_get_glyph_positions(m_handle.get(), &glyph_count)
				}
			};
		}

	private:
		hb_buffer_handle m_handle;
		bool m_clear_before_append{false};
	};

}

#endif