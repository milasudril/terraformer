#ifndef TERRAFORMER_GUI_HEIGHTMAP_HPP
#define TERRAFORMER_GUI_HEIGHTMAP_HPP

#include "./elevation_color_map.hpp"
#include "./domain_size.hpp"
#include "./plain.hpp"
#include "./rolling_hills.hpp"

#include "ui/widgets/form.hpp"
#include "ui/widgets/heatmap_view.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/knob.hpp"
#include "ui/widgets/float_input.hpp"
#include "ui/value_maps/log_value_map.hpp"

#include "lib/pixel_store/image.hpp"
#include "lib/generators/heightmap/heightmap.hpp"

namespace terraformer::app
{
	template<class View>
	struct heightmap_part_form_field
	{
		std::u8string_view label;
		std::reference_wrapper<grayscale_image const> value_reference;
		bool expand_layout_cell;
		bool maximize_widget;

		using input_widget_type = View;
	};

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

	struct heatmap_view_attributes
	{
		level_curves_descriptor level_curves;
	};

	struct heatmap_view_attributes_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	auto& bind(std::u8string_view field_name, heatmap_view_attributes& field_value, ui::widgets::form& form)
	{
		auto& settings_form = form.create_widget(
			heatmap_view_attributes_form_field{
				.label = field_name
			}
		);

		bind(u8"Level curves", field_value.level_curves, settings_form);

		return settings_form;
	}

	enum class heightmap_active_view:size_t{
		heatmap
	};

	struct heightmap_view_descriptor
	{
		std::reference_wrapper<grayscale_image const> data;
		heightmap_active_view active_view = heightmap_active_view::heatmap;
		heatmap_view_attributes heatmap_presentation_attributes;
	};

	struct heightmap_chart_form_field
	{
		std::u8string_view label;
		bool expand_layout_cell;
		using input_widget_type = ui::widgets::form;
	};

	struct heatmap_view_descriptor
	{
		explicit heatmap_view_descriptor(heightmap_view_descriptor& main_view):
			data{main_view.data},
			presentation_attributes{main_view.heatmap_presentation_attributes}
		{}

		std::reference_wrapper<grayscale_image const> data;
		std::reference_wrapper<heatmap_view_attributes> presentation_attributes;
	};

	auto& bind(
		std::u8string_view field_name,
		heatmap_view_descriptor const& field_value,
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
				.value_reference = field_value.data,
				.expand_layout_cell = true,
				.maximize_widget = true
			},
			u8"Elevation/m",
			terraformer::global_elevation_map,
			terraformer::get_elevation_color_lut()
		);

		auto& settings_form = bind(u8"Settings", field_value.presentation_attributes.get(), ret);
		settings_form.on_content_updated([&level_curves = field_value.presentation_attributes.get().level_curves, &imgview](auto&&...){
			if(level_curves.visible)
			{ imgview.show_level_curves(); }
			else
			{ imgview.hide_level_curves(); }

			imgview.set_level_curve_interval(level_curves.interval);
		});

		ret.set_refresh_function([image = field_value.data, &imgview](){
			imgview.show_image(image.get().pixels());
		});

		return ret;
	}

	auto& bind(std::u8string_view field_name, heightmap_view_descriptor& field_value, ui::widgets::form& form)
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
			heatmap_view_descriptor{field_value},
			ret
		);

		ret.set_refresh_function([&heatmap](){
			heatmap.refresh();
		});

		return ret;
	}
	struct heightmap_generator_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	auto& bind(std::u8string_view field_name, heightmap_generator_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			heightmap_generator_form_field{
				.label = field_name
			}
		);

		bind(u8"Plain", field_value.plain, ret);
		bind(u8"Rolling hills", field_value.rolling_hills, ret);

		return ret;
	}

	struct heightmap_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	auto& bind(std::u8string_view field_name, heightmap_descriptor& field_value, ui::widgets::form& form)
	{
		auto& ret = form.create_widget(
			domain_size_form_field{
				.label = field_name,
			}
		);

		bind(u8"Domain size", field_value.domain_size, ret);

		bind(u8"Generators", field_value.generators, ret);

		return ret;
	}
}

#endif