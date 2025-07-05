//@	{"target": {"name":"colorbar.o", "rel":"implementation"}}

#include "./colorbar.hpp"
#include "ui/drawing_api/image_generators.hpp"
#include "lib/common/utils.hpp"
#include "ui/main/widget_geometry.hpp"

#include <algorithm>

void terraformer::ui::widgets::colorbar::update_colorbar()
{
	auto const w = static_cast<uint32_t>(m_size[0] + 0.5f);
	auto const h_label = m_size[1]/static_cast<float>(std::size(m_labels).get());
	auto const h = static_cast<uint32_t>(m_size[1] + 0.5f - h_label);
	image output_image{w, h};

	for(uint32_t y = 0; y != h; ++y)
	{
		for(uint32_t x = 0; x != w; ++x)
		{
			auto const grayscale_val = static_cast<float>(h - y - 1)/(static_cast<float>(h - 1));
			output_image(x, y) = m_color_map(grayscale_val);
		}
	}
	m_image = std::move(output_image);
}

void terraformer::ui::widgets::colorbar::update_frame()
{
	auto const w = static_cast<uint32_t>(m_size[0] + 0.5f);
	auto const h_label = m_size[1]/static_cast<float>(std::size(m_labels).get());
	auto const h = static_cast<uint32_t>(m_size[1] + 0.5f - h_label);
	auto output_frame = generate(
		drawing_api::flat_rectangle{
			.domain_size = span_2d_extents {
				.width = w,
				.height = h
			},
			.origin_x = 0u,
			.origin_y = 0u,
			.width = w,
			.height = h,
			.border_thickness = m_border_thickness,
			.border_color = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
			.fill_color = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}
		}
	);

	auto const marker_length = static_cast<uint32_t>(m_marker_length);
	auto const element_indices = m_labels.element_indices();
	for(auto index : element_indices)
	{
		auto const intensity = static_cast<float>(index - element_indices.front())/
			static_cast<float>(element_indices.back() - element_indices.front());
		auto const y = static_cast<uint32_t>((1.0f - intensity)*static_cast<float>(h - 1) + 0.5f);
		for(uint32_t x = 0; x != w; ++x)
		{
			if(x < marker_length || x >= w - marker_length)
			{ output_frame(x, y) = rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}; }
			else
			{ output_frame(x, y) = rgba_pixel{0.0f, 0.0f, 0.0f, 0.0f}; }
		}
	}

	m_frame = std::move(output_frame);
}

terraformer::ui::main::widget_layer_stack terraformer::ui::widgets::colorbar::prepare_for_presentation(main::graphics_backend_ref backend)
{
	auto const null_texture = m_null_texture->get_backend_resource(backend).get();
	auto const h_label = 0.875f*static_cast<float>(m_labels.front().text_height());
	auto const scale_offset = displacement{0.0f, h_label, 0.0f};

	return main::widget_layer_stack{
		.background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.sel_bg_mask = main::widget_layer_mask{
			.offset = displacement{},
			.texture = null_texture
		},
		.selection_background = main::widget_layer{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		},
		.foreground = main::widget_layer{
			.offset = scale_offset,
			.rotation = geosimd::turn_angle{},
			.texture = m_image.get_backend_resource(backend).get(),
			.tints = std::array<rgba_pixel, 4>{
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f},
				rgba_pixel{1.0f, 1.0f, 1.0f, 1.0f}
			}
		},
		.frame = main::widget_layer{
			.offset = scale_offset,
			.rotation = geosimd::turn_angle{},
			.texture = m_frame.get_backend_resource(backend).get(),
			.tints = std::array{m_fg_tint, m_fg_tint, m_fg_tint, m_fg_tint}
		},
		.input_marker{
			.offset = displacement{},
			.rotation = geosimd::turn_angle{},
			.texture = null_texture,
			.tints = std::array<rgba_pixel, 4>{}
		}
	};
}

void terraformer::ui::widgets::colorbar::update_labels()
{
	auto const value_map_ptr = m_value_map.get().get_pointer();
	auto const to_value = m_value_map.get().get_vtable().to_value;
	auto const label_indices = m_labels.element_indices();
	for(auto k :label_indices)
	{
		auto const intensity = static_cast<float>(label_indices.back() - k)
			/static_cast<float>(label_indices.back() - label_indices.front());
		auto const value = to_value(value_map_ptr, intensity);
		// TODO: siformat should return u8
		m_labels[k].value(reinterpret_cast<char8_t const*>(siformat(value, 3).c_str()));
	}
}

terraformer::box_size terraformer::ui::widgets::colorbar::confirm_size(box_size size_in)
{
	if(size_in != m_size)
	{
		auto const w_max = static_cast<float>(
			std::ranges::max_element
				(m_labels, [](auto const& a, auto const& b){
				return a.text_width() < b.text_width();
			})->text_width()
		);

		size_in[0] = w_max + 3.0f*m_marker_length;

		auto const h_label = m_size[1]/static_cast<float>(std::size(m_labels).get());
		size_in = max(size_in, box_size{0.0f, 2.0f + h_label, 0.0f});

		m_size = size_in;
		update_colorbar();
		update_frame();
	}
	return size_in;
}

void terraformer::ui::widgets::colorbar::init()
{
	set_label_count(13);
	layout.params().no_outer_margin = false;
	layout.params().margin_y = 0.0f;
}

void terraformer::ui::widgets::colorbar::set_label_count(size_t new_count)
{
	single_array temp{array_size<label>{new_count}};
	for(auto& item : temp)
	{ item.set_margin(0.0f); }
	clear();
	m_labels = std::move(temp);
	// TODO: Better to set text inside the temp array
	update_labels();
	for(auto& item : m_labels)
	{ append(std::ref(item), main::widget_geometry{}); }

	for(size_t k = 0; k != new_count; ++k)
	{ layout.set_cell_size(k, layouts::table::cell_size::expand{}); }
}