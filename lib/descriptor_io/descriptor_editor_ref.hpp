#ifndef TERRAFORMER_DESCRIPTOR_EDITOR_REF_HPP
#define TERRAFORMER_DESCRIPTOR_EDITOR_REF_HPP

#include "lib/common/interval.hpp"
#include "lib/common/unique_resource.hpp"
#include "lib/common/value_map.hpp"
#include "lib/value_maps/affine_value_map.hpp"

#include <geosimd/angle.hpp>
#include <initializer_list>
#include <string_view>

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
		enum class widget_orientation{horizontal, vertical};

		template<class DescriptorEditor, class DescriptorEditorTraits>
		explicit descriptor_editor_ref(DescriptorEditor& editor, std::type_identity<DescriptorEditorTraits>):
			m_handle{std::ref(editor), std::type_identity<DescriptorEditorTraits>{}}
		{}

		struct table_descriptor
		{
			widget_orientation orientation;
			std::initializer_list<char8_t const*> field_names;
		};

		descriptor_table_editor_ref
		create_table(std::u8string_view label, table_descriptor&& table_params) const
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.create_table(pointer, label, std::move(table_params));
		}

		void append_pending_widgets()
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.append_pending_widgets(pointer);
		}

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

		void create_float_input(std::u8string_view label, assigner<float> value, knob_descriptor&& knob_params)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.create_float_assigner_input_knob(pointer, label, value, std::move(knob_params));
		}

		struct form_descriptor
		{
			widget_orientation orientation;
			size_t extra_fields_per_row = 0;
		};

		descriptor_editor_ref create_form(std::u8string_view label, form_descriptor&& form_params)
		{
			auto const vt = m_handle.get_vtable();
			auto const pointer = m_handle.get_pointer();
			return vt.create_form(pointer, label, std::move(form_params));
		}

	private:
		struct vtable
		{
			template<class DescriptorEditor, class DescriptorEditorTraits>
			constexpr explicit vtable(std::type_identity<DescriptorEditor>, std::type_identity<DescriptorEditorTraits>):
				create_table{[](void* handle, std::u8string_view label, table_descriptor&& table_params){
					return DescriptorEditorTraits::create_table(
						*static_cast<DescriptorEditor*>(handle),
						label,
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
				append_pending_widgets{[](void* handle){
					return DescriptorEditorTraits::append_pending_widgets(*static_cast<DescriptorEditor*>(handle));
				}}
			{}

			descriptor_table_editor_ref (*create_table)(void*, std::u8string_view, table_descriptor&&);
			descriptor_editor_ref (*create_form)(void*, std::u8string_view, form_descriptor&&);
			void (*create_float_input_knob)(void*, std::u8string_view, float&, knob_descriptor&&);
			void (*create_float_assigner_input_knob)(void*, std::u8string_view, assigner<float>, knob_descriptor&&);
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