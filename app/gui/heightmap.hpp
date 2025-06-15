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

	void bind(level_curves_descriptor& field_value, ui::widgets::form& parent)
	{
		parent.create_widget(
			level_curves_visible_form_field{
				.label = u8"Visible",
				.value_reference = std::ref(field_value.visible),
				.label_on_input_field = true
			}
		);

		parent.create_widget(
			level_curves_interval_form_field{
				.label = u8"Interval/m",
				.value_reference = std::ref(field_value.interval)
			},
			terraformer::ui::widgets::knob{
				terraformer::ui::value_maps::log_value_map{1.0f, 1024.0f, 2.0f}
			}
		)
		.set_textbox_placeholder_string(u8"9999.9999");
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

	void bind(heatmap_view_attributes& field_value, ui::widgets::form& parent)
	{
		auto& level_curves_form = parent.create_widget(
			level_curves_descriptor_form_field{
				.label = u8"Level curves"
			},
			terraformer::ui::main::widget_orientation::horizontal
		);

		bind(field_value.level_curves, level_curves_form);
	}

	struct heightmap_view_descriptor
	{
		std::reference_wrapper<grayscale_image const> data;
		heatmap_view_attributes heatmap_presentation_attributes;
	};

	struct heightmap_heatmap_form_field
	{
		std::u8string_view label;
		bool expand_layout_cell;
		using input_widget_type = ui::widgets::form;
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

	void bind(heatmap_view_descriptor const& field_value, ui::widgets::form& parent)
	{
		auto& imgview = parent.create_widget(
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

		auto& settings_form = parent.create_widget(
			heatmap_view_attributes_form_field{
				.label = u8"Settings"
			}
		);
		bind(field_value.presentation_attributes.get(), settings_form);
		settings_form.on_content_updated([&level_curves = field_value.presentation_attributes.get().level_curves, &imgview](auto&&...){
			if(level_curves.visible)
			{ imgview.show_level_curves(); }
			else
			{ imgview.hide_level_curves(); }

			imgview.set_level_curve_interval(level_curves.interval);
		});

		parent.set_refresh_function([image = field_value.data, &imgview](){
			imgview.show_image(image.get().pixels());
		});
	}

	struct heightmap_xsections_form_field
	{
		std::u8string_view label;
		bool expand_layout_cell;
		using input_widget_type = ui::widgets::form;
	};

	void bind(heightmap_view_descriptor& field_value, ui::widgets::form& parent)
	{
		auto& heatmap = parent.create_widget(
			heightmap_heatmap_form_field{
				.label = u8"Heatmap view",
				.expand_layout_cell = true
			}
		);
		bind(heatmap_view_descriptor{field_value}, heatmap);

		auto& xsections = parent.create_widget(
			heightmap_xsections_form_field{
				.label = u8"Cross-sections",
				.expand_layout_cell = true
			}
		);
		bind(heatmap_view_descriptor{field_value}, xsections);

		parent.set_refresh_function([&heatmap, &xsections](){
			heatmap.refresh();
			xsections.refresh();
		});
	}

	struct heightmap_generator_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	void bind(heightmap_generator_descriptor& field_value, ui::widgets::form& parent)
	{
		auto& plain = parent.create_widget(
			plain_form_field{
				.label = u8"Plain"
			}
		);
		bind(field_value.plain, plain);


		auto& rolling_hills = parent.create_widget(
			rolling_hills_descriptor_form_field{
				.label = u8"Rolling hills"
			}
		);
		bind(field_value.rolling_hills, rolling_hills);
	}

	struct heightmap_form_field
	{
		std::u8string_view label;
		using input_widget_type = ui::widgets::form;
	};

	void bind(heightmap_descriptor& field_value, ui::widgets::form& parent)
	{
		auto& domain_size_form = parent.create_widget(
			domain_size_form_field{
				.label = u8"Domain size",
			}
		);
		bind(field_value.domain_size, domain_size_form);

		auto& generators = parent.create_widget(
			heightmap_generator_form_field{
				.label = u8"Generators"
			}
		);
		bind(field_value.generators, generators);
	}
}

#endif