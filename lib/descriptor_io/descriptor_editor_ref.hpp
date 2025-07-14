#ifndef TERRAFORMER_DESCRIPTOR_EDITOR_REF_HPP
#define TERRAFORMER_DESCRIPTOR_EDITOR_REF_HPP

#include "lib/common/interval.hpp"
#include "lib/common/unique_resource.hpp"
#include "lib/common/value_map.hpp"
#include "lib/value_maps/affine_value_map.hpp"

#include <geosimd/angle.hpp>
#include <initializer_list>
#include <string_view>
#include <string>

namespace terraformer
{
	class descriptor_editor_ref;

	class descriptor_table_editor_ref
	{
	public:
		enum class widget_orientation{horizontal, vertical};

		template<class DescriptorEditor, class DescriptorEditorTraits>
		explicit descriptor_table_editor_ref(DescriptorEditor& editor, std::type_identity<DescriptorEditorTraits>):
			m_handle{std::ref(editor), std::type_identity<DescriptorEditorTraits>{}}
		{}

		inline descriptor_editor_ref add_record(std::u8string_view label) const;

	private:
		struct vtable
		{
			template<class DescriptorEditor, class DescriptorEditorTraits>
			constexpr explicit vtable(std::type_identity<DescriptorEditor>, std::type_identity<DescriptorEditorTraits>);

			descriptor_editor_ref (*add_record)(void*, std::u8string_view);
		};

		resource_reference<vtable> m_handle;
	};

	class descriptor_editor_ref
	{
	public:
		enum class widget_orientation{deduce, horizontal, vertical};

		template<class DescriptorEditor, class DescriptorEditorTraits>
		explicit descriptor_editor_ref(DescriptorEditor& editor, std::type_identity<DescriptorEditorTraits>):
			m_handle{std::ref(editor), std::type_identity<DescriptorEditorTraits>{}}
		{}
		
		struct field_descriptor
		{
			std::u8string_view label;
			bool expand_layout_cell = false;
		};

		struct table_descriptor
		{
			widget_orientation orientation = widget_orientation::deduce;
			std::initializer_list<char8_t const*> field_names{};
		};

		descriptor_table_editor_ref
		create_table(field_descriptor const& field_info, table_descriptor&& table_params) const
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.create_table(pointer, field_info, std::move(table_params));
		}

		struct form_descriptor
		{
			widget_orientation orientation = widget_orientation::deduce;
			size_t extra_fields_per_row = 0;
		};

		descriptor_editor_ref create_form(std::u8string_view label, form_descriptor&& form_params)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.create_form(pointer, label, std::move(form_params));
		}

		template<class Rhs>
		class assigner
		{
		public:
			template<class Lhs>
			requires(!std::is_same_v<Rhs, Lhs>)
			explicit assigner(Lhs& lhs):
				m_lhs{&lhs},
				m_do_assign{[](void* lhs, Rhs&& rhs){
					*static_cast<Lhs*>(lhs) = Lhs{std::move(rhs)};
				}},
				m_get_value{[](void const* lhs) -> Rhs{
					return static_cast<Rhs>(*static_cast<Lhs const*>(lhs));
				}}
			{}

			template<class T>
			assigner const& operator=(T&& other) const
			{
				m_do_assign(m_lhs, Rhs{std::forward<T>(other)});
				return *this;
			}

			assigner const& get() const
			{ return *this; }

			operator Rhs() const
			{ return m_get_value(m_lhs); }

		private:
			void* m_lhs;
			void (*m_do_assign)(void*, Rhs&&);
			Rhs (*m_get_value)(void const*);
		};

		struct knob_descriptor
		{
			type_erased_value_map value_map = type_erased_value_map{value_maps::affine_value_map{0.0f, 1.0f}};
			std::u8string_view textbox_placeholder_string;
			std::optional<closed_closed_interval<geosimd::turn_angle>> visual_angle_range;
		};

		void create_float_input(std::u8string_view label, float& value, knob_descriptor&& knob_params)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			vt.create_float_input_knob(pointer, label, value, std::move(knob_params));
		}

		void create_float_input(std::u8string_view label, assigner<float> value, knob_descriptor&& knob_params)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.create_float_assigner_input_knob(pointer, label, value, std::move(knob_params));
		}

		struct slider_descriptor
		{
			type_erased_value_map value_map = type_erased_value_map{value_maps::affine_value_map{0.0f, 1.0f}};
			std::u8string_view textbox_placeholder_string;
			widget_orientation orientation;
		};

		void create_float_input(std::u8string_view label, float& value, slider_descriptor&& knob_params)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			vt.create_float_input_slider(pointer, label, value, std::move(knob_params));
		}

		struct single_line_text_input_descriptor
		{
			std::u8string_view textbox_placeholder_string;
		};

		void create_string_input(
			std::u8string_view label,
			std::u8string& value,
			single_line_text_input_descriptor&& text_input_params
		)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer =m_handle.get_pointer();
			vt.create_string_input_single_line_text_input(pointer, label, value, std::move(text_input_params));
		}

		void create_rng_seed_input(std::u8string_view label, std::array<std::byte, 16>& value)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			vt.create_rng_seed_input(pointer, label, value);
		}

		struct range_input_descriptor
		{
			type_erased_value_map value_map = type_erased_value_map{value_maps::affine_value_map{0.0f, 1.0f}};
			std::u8string_view textbox_placeholder_string;
		};

		void create_range_input(
			std::u8string_view label,
			closed_closed_interval<float>& value,
			range_input_descriptor&& range_input_params
		)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			vt.create_range_input(pointer, label, value, std::move(range_input_params));
		}

		void append_pending_widgets()
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.append_pending_widgets(pointer);
		}

	private:
		struct vtable
		{
			template<class DescriptorEditor, class DescriptorEditorTraits>
			constexpr explicit vtable(std::type_identity<DescriptorEditor>, std::type_identity<DescriptorEditorTraits>):
				create_table{[](void* handle, field_descriptor const& field_info, table_descriptor&& table_params){
					return DescriptorEditorTraits::create_table(
						*static_cast<DescriptorEditor*>(handle),
						field_info,
						std::move(table_params)
					);
				}},
				create_form{[](void* handle, std::u8string_view label, form_descriptor&& form_params){
					return DescriptorEditorTraits::create_form(
						*static_cast<DescriptorEditor*>(handle),
						label,
						std::move(form_params)
					);
				}},
				create_float_input_knob{[](void* handle, std::u8string_view label, float& value, knob_descriptor&& knob_params) {
					DescriptorEditorTraits::create_float_input(*static_cast<DescriptorEditor*>(handle), label, std::ref(value), std::move(knob_params));
				}},
				create_float_assigner_input_knob{[](void* handle, std::u8string_view label, assigner<float> value, knob_descriptor&& knob_params) {
					DescriptorEditorTraits::create_float_input(*static_cast<DescriptorEditor*>(handle), label, value, std::move(knob_params));
				}},
				create_float_input_slider{[](void* handle, std::u8string_view label, float& value, slider_descriptor&& slider_params) {
					DescriptorEditorTraits::create_float_input(*static_cast<DescriptorEditor*>(handle), label, std::ref(value), std::move(slider_params));
				}},
				create_string_input_single_line_text_input{[](void* handle, std::u8string_view label, std::u8string& value, single_line_text_input_descriptor&& text_input_params){
					DescriptorEditorTraits::create_string_input(*static_cast<DescriptorEditor*>(handle), label, std::ref(value), std::move(text_input_params));
				}},
				create_rng_seed_input{[](void* handle, std::u8string_view label, std::array<std::byte, 16>& value){
					DescriptorEditorTraits::create_rng_seed_input(*static_cast<DescriptorEditor*>(handle), label, value);
				}},
				create_range_input{[](void* handle, std::u8string_view label, closed_closed_interval<float>& value, range_input_descriptor&& range_input_params){
					DescriptorEditorTraits::create_range_input(*static_cast<DescriptorEditor*>(handle), label, value, std::move(range_input_params));
				}},
				append_pending_widgets{[](void* handle){
					return DescriptorEditorTraits::append_pending_widgets(*static_cast<DescriptorEditor*>(handle));
				}}
			{}

			descriptor_table_editor_ref (*create_table)(void*, field_descriptor const&, table_descriptor&&);
			descriptor_editor_ref (*create_form)(void*, std::u8string_view, form_descriptor&&);
			void (*create_float_input_knob)(void*, std::u8string_view, float&, knob_descriptor&&);
			void (*create_float_assigner_input_knob)(void*, std::u8string_view, assigner<float>, knob_descriptor&&);
			void (*create_float_input_slider)(void*, std::u8string_view, float&, slider_descriptor&&);
			void (*create_string_input_single_line_text_input)(void*, std::u8string_view, std::u8string&, single_line_text_input_descriptor&&);
			void (*create_rng_seed_input)(void*, std::u8string_view, std::array<std::byte, 16>&);
			void (*create_range_input)(void*, std::u8string_view, closed_closed_interval<float>&, range_input_descriptor&&);
			void (*append_pending_widgets)(void*);
		};

		resource_reference<vtable> m_handle;
	};

	inline descriptor_editor_ref descriptor_table_editor_ref::add_record(std::u8string_view label) const
	{
		auto const vt = m_handle.get_vtable();
		auto const pointer = m_handle.get_pointer();
		return vt.add_record(pointer, label);
	}

	template<class DescriptorEditor, class DescriptorEditorTraits>
	constexpr descriptor_table_editor_ref::vtable::vtable(
		std::type_identity<DescriptorEditor>,
		std::type_identity<DescriptorEditorTraits>
	):
		add_record{[](
			void* handle,
			std::u8string_view label
		){
			return DescriptorEditorTraits::add_record(
				*static_cast<DescriptorEditor*>(handle),
				label
			);
		}}
	{}
}

#endif
