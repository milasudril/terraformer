#ifndef TERRAFORMER_UI_WIDGETS_RNG_SEED_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_RNG_SEED_INPUT_HPP

#include "./widget_group.hpp"
#include "./text_to_bytearray_input.hpp"
#include "./button.hpp"
#include "ui/layouts/table.hpp"
#include "ui/main/widget_collection.hpp"
#include "lib/common/rng.hpp"

namespace terraformer::ui::widgets
{
	template<size_t NumBytes>
	class rng_seed_input:private widget_group<layouts::table>
	{
	public:
		using widget_group::handle_event;
		using widget_group::prepare_for_presentation;
		using widget_group::theme_updated;
		using widget_group::get_children;
		using widget_group::get_layout;
		using widget_group::compute_size;
		using widget_group::confirm_size;

		explicit rng_seed_input(
			iterator_invalidation_handler_ref iihr,
			main::widget_orientation orientation = main::widget_orientation::horizontal
		):
			widget_group{
				iihr,
				2u,
				orientation == main::widget_orientation::vertical?
					layouts::table::cell_order::column_major:
					layouts::table::cell_order::row_major
			}
		{ init(); }

		template<class Function>
		auto& on_value_changed(Function&& f)
		{
			m_on_value_changed = std::forward<Function>(f);
			return *this;
		}

		auto value() const
		{ return m_textbox.value(); }

		auto& value(std::array<std::byte, NumBytes> new_val)
		{
			m_textbox.value(new_val);
			return *this;
		}

	private:
		text_to_bytearray_input<NumBytes> m_textbox;
		button m_trigger;
		main::widget_user_interaction_handler<rng_seed_input<NumBytes>> m_on_value_changed{no_operation_tag{}};

		void init()
		{
			append(std::ref(m_textbox), terraformer::ui::main::widget_geometry{});
			append(std::ref(m_trigger), terraformer::ui::main::widget_geometry{});
			m_textbox.on_value_changed([this]<class ... Args>(auto const&, Args&&... args){
				m_on_value_changed(*this, std::forward<Args>(args)...);
			});
			m_trigger
				.on_activated([this]<class ... Args>(auto const&, Args&&... args){
					m_textbox.value(random_bit_source{system_rng_path}.get<std::array<std::byte, NumBytes>>());
					m_on_value_changed(*this, std::forward<Args>(args)...);
				})
				.text(u8"Pick new");
			layout.params().no_outer_margin = true;
			is_transparent = true;
		}
	};
}

#endif