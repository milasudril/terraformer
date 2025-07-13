#ifndef TERRAFORMER_APP_DESCRIPTOR_EDITOR_HPP
#define TERRAFORMER_APP_DESCRIPTOR_EDITOR_HPP

#include "lib/common/interval.hpp"
#include "lib/descriptor_io/descriptor_editor_ref.hpp"
#include "ui/main/widget.hpp"
#include "ui/widgets/form.hpp"
#include "ui/widgets/table.hpp"
#include "ui/widgets/rng_seed_input.hpp"
#include "ui/widgets/interval_input.hpp"

namespace terraformer::app
{
	struct descriptor_editor_traits
	{
		struct table_descriptor
		{
			using input_widget_type = ui::widgets::table;
			std::u8string_view label;
		};

		template<class Parent>
		static descriptor_table_editor_ref create_table(
			Parent& parent,
			std::u8string_view label,
			descriptor_editor_ref::table_descriptor&& params
		)
		{
			if(params.orientation == descriptor_editor_ref::widget_orientation::deduce)
			{
				return descriptor_table_editor_ref{
					parent.create_widget(table_descriptor{.label = label}, params.field_names),
					std::type_identity<descriptor_editor_traits>{}
				};
			}

			return descriptor_table_editor_ref{
				parent.create_widget(
					table_descriptor{.label = label},
					params.orientation == descriptor_editor_ref::widget_orientation::horizontal?
						ui::main::widget_orientation::horizontal :
						ui::main::widget_orientation::vertical,
					params.field_names
				),
				std::type_identity<descriptor_editor_traits>{}
			};
		}

		struct record_decriptor
		{
			std::u8string_view label;
		};

		template<class Parent>
		static descriptor_editor_ref add_record(Parent& parent, std::u8string_view label)
		{
			return descriptor_editor_ref{
				parent.add_record(record_decriptor{.label = label}),
				std::type_identity<descriptor_editor_traits>{}
			};
		}

		struct form_descriptor
		{
			std::u8string_view label;
			using input_widget_type = ui::widgets::form;
		};

		template<class Parent>
		static descriptor_editor_ref create_form(
			Parent& parent,
			std::u8string_view label,
			descriptor_editor_ref::form_descriptor&& params
		)
		{
			if(params.orientation == descriptor_editor_ref::widget_orientation::deduce)
			{
				return descriptor_editor_ref{
					parent.create_widget(form_descriptor{.label = label}, params.extra_fields_per_row),
					std::type_identity<descriptor_editor_traits>{}
				};
			}

			return descriptor_editor_ref{
				parent.create_widget(
					form_descriptor{.label = label},
					params.orientation == descriptor_editor_ref::widget_orientation::horizontal?
						ui::main::widget_orientation::horizontal :
						ui::main::widget_orientation::vertical,
					params.extra_fields_per_row
				),
				std::type_identity<descriptor_editor_traits>{}
			};
		}

		template<class FloatWrapper>
		struct knob_descriptor
		{
			std::u8string_view label;
			FloatWrapper value_reference;
			using input_widget_type = ui::widgets::float_input<ui::widgets::knob>;
		};

		template<class Parent, class FloatWrapper>
		static void create_float_input(Parent& parent, std::u8string_view label, FloatWrapper value, descriptor_editor_ref::knob_descriptor&& params)
		{
			auto& widget = parent.create_widget(
				knob_descriptor{
					.label = label,
					.value_reference = value
				},
				terraformer::ui::widgets::knob{std::move(params.value_map)}
			);

			if(params.visual_angle_range.has_value())
			{ widget.input_widget().visual_angle_range(*params.visual_angle_range); }

			if(params.textbox_placeholder_string.data() != nullptr)
			{ widget.set_textbox_placeholder_string(params.textbox_placeholder_string); }
		}


		template<size_t N>
		struct rng_seed_input_descriptor
		{
			std::u8string_view label;
			std::reference_wrapper<std::array<std::byte, N>> value_reference;
			using input_widget_type = ui::widgets::rng_seed_input<N>;
		};

		template<class Parent, size_t N>
		static void create_rng_seed_input(Parent& parent, std::u8string_view label, std::array<std::byte, N>& value)
		{
			parent.create_widget(
				rng_seed_input_descriptor<N>{
					.label = label,
					.value_reference = value
				}
			);
		}

		struct range_input_descriptor
		{
			std::u8string_view label;
			std::reference_wrapper<closed_closed_interval<float>> value_reference;
			bool expand_layout_cell;
			using input_widget_type = ui::widgets::interval_input;
		};

		template<class Parent>
		static void create_range_input(
			Parent& parent,
			std::u8string_view label,
			closed_closed_interval<float>& value,
			descriptor_editor_ref::range_input_descriptor&& params
		)
		{
			auto& widget = parent.create_widget(
				range_input_descriptor{
					.label = label,
					.value_reference = value,
					.expand_layout_cell = true
				},
				std::move(params.value_map)
			);

			if(params.textbox_placeholder_string.data() != nullptr)
			{ widget.set_textbox_placeholder_string(params.textbox_placeholder_string); }
		}

		template<class Obj>
		static void append_pending_widgets(Obj& obj)
		{ obj.append_pending_widgets(); }
	};
}

#endif
