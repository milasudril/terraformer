#ifndef TERRAFORMER_GUI_HEIGHTMAP_HPP
#define TERRAFORMER_GUI_HEIGHTMAP_HPP

#include "./elevation_color_map.hpp"

#include "ui/widgets/form.hpp"
#include "ui/widgets/widget_canvas.hpp"
#include "ui/widgets/heatmap_view.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"
#include "ui/value_maps/log_value_map.hpp"

#include "lib/pixel_store/image.hpp"

namespace terraformer::app
{
	struct level_curves_descriptor
	{
		float interval = 100.0f;
		bool visible = true;
	};

	struct level_curves_descriptor_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	struct level_curves_visible_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<bool> value_reference;
		bool label_on_input_field;
		using input_widget_type = ui::widgets::toggle_button;
	};

	struct level_curves_interval_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<float> value_reference;
		using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
	};

	auto& bind(std::u8string_view field_name, level_curves_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			level_curves_descriptor_form_field{
				.label = field_name
			},
			terraformer::ui::main::widget_orientation::horizontal
		);

		ret.create_widget(
			level_curves_visible_form_field{
				.label = u8"Visible",
				.value_reference = std::ref(field_value.visible),
				.label_on_input_field = true
			}
		);

		ret.create_widget(
			level_curves_interval_form_field{
				.label = u8"Interval/m",
				.value_reference = std::ref(field_value.interval)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1.0f, 1024.0f, 2.0f}
			}
		);

		return ret;
	}

	struct heightmap_descriptor
	{
		grayscale_image data;
		level_curves_descriptor level_curves;
	};

	struct heightmap_chart_form_field
	{
		std::u8string_view label;
		bool expand_layout_cell;
		using input_widget_type = ui::widgets::form;
	};

	struct heightmap_data_area_form_field
	{
		std::u8string_view label;
		bool expand_layout_cell;
		using input_widget_type = ui::widgets::widget_canvas;
	};

	template<class View>
	struct heightmap_part_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<grayscale_image const> value_reference;
		bool expand_layout_cell;
		bool maximize_widget;

		using input_widget_type = View;
	};

	struct level_curves_form_field
	{
		std::u8string_view label;
		using input_widget_type = terraformer::ui::widgets::form;
	};

	template<class Tag, class Value>
	struct tagged_value
	{
		using tag_type = Tag;
		Value value;
	};

	template<class Tag, class Value>
	constexpr auto make_tagged_value(Value&& val)
	{
		return tagged_value<Tag, Value>{std::forward<Value>(val)};
	}

	auto& bind(
		std::u8string_view field_name,
		tagged_value<terraformer::ui::widgets::heatmap_view, heightmap_descriptor&> field_value,
		ui::widgets::form& form
	)
	{
		auto& ret = form.create_widget(
			heightmap_chart_form_field{
				.label = field_name,
				.expand_layout_cell = true
			}
		);

		auto& imgview = ret.create_widget(
			heightmap_part_form_field<terraformer::ui::widgets::heatmap_view>{
				.label = u8"Output",
				.value_reference = field_value.value.data,
				.expand_layout_cell = true,
				.maximize_widget = true
			},
			u8"Elevation/m",
			terraformer::global_elevation_map,
			terraformer::get_elevation_color_lut()
		);

		auto& settings_form = ret.create_widget(
			heightmap_chart_form_field{
				.label = u8"Settings",
				.expand_layout_cell = false
			}
		);

		auto& level_curves = bind(u8"Level curves", field_value.value.level_curves, settings_form);
		level_curves.on_content_updated([&level_curves = field_value.value.level_curves, &imgview](auto&&...){
			if(level_curves.visible)
			{ imgview.show_level_curves(); }
			else
			{ imgview.hide_level_curves(); }

			imgview.set_level_curve_interval(level_curves.interval);
		});

		ret.set_refresh_function([&image = field_value.value.data, &imgview](){
			imgview.show_image(std::as_const(image).pixels());
		});

		return ret;
	}

	auto& bind(std::u8string_view field_name, heightmap_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			heightmap_chart_form_field{
				.label = field_name,
				.expand_layout_cell = true
			},
			terraformer::ui::main::widget_orientation::horizontal
		);

		auto& heatmap = bind(
			u8"Heatmap view",
			make_tagged_value<terraformer::ui::widgets::heatmap_view>(field_value),
			ret
		);

		ret.set_refresh_function([&heatmap](){
			heatmap.refresh();
		});

		return ret;
	}
}

#endif