#ifndef TERRAFORMER_UI_WIDGETS_FLOAT_INPUT_CONTROLLER_HPP
#define TERRAFORMER_UI_WIDGETS_FLOAT_INPUT_CONTROLLER_HPP

#include "ui/main/builtin_command_id.hpp"
#include "ui/main/widget.hpp"

#include "lib/common/move_only_function.hpp"
#include "lib/common/bounded_value.hpp"
#include "lib/common/interval.hpp"
#include "lib/common/value_map.hpp"
#include "lib/value_maps/affine_value_map.hpp"

namespace terraformer::ui::widgets
{
	template<class Derived>
	class float_input_controller:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;
		using internal_value_type = bounded_value<closed_closed_interval{0.0f, 1.0f}, 0.0f>;

		enum class state{released, handle_grabbed};

		float_input_controller() = default;

		template<class ValueMap>
		requires (!std::is_same_v<std::remove_cvref_t<ValueMap>, float_input_controller>)
		explicit float_input_controller(ValueMap&& vm):
			m_value_map{std::forward<ValueMap>(vm)}
		{ value(0.0f); }

		template<class ValueMap, class... Args>
		requires (std::is_same_v<std::remove_cvref_t<ValueMap>, terraformer::type_erased_value_map>)
		explicit float_input_controller(std::in_place_type_t<ValueMap>, Args&&... args):
			m_value_map{std::in_place_type_t<ValueMap>{}, std::forward<Args>(args)...}
		{ value(0.0f); }

		void handle_event(
			main::cursor_motion_event const& cme,
			main::window_ref window,
			main::ui_controller controller
		)
		{
			if(m_state_current == state::handle_grabbed)
			{
				m_value = derived().to_internal_value(cme.where);
				m_on_value_changed(derived(), window, controller);
			}
		}

		void value(float new_val)
		{ m_value = internal_value_type{from_value(new_val), clamp_tag{}}; }

		float value() const
		{ return to_value(m_value); }

		template<class Function>
		Derived& on_value_changed(Function&& func)
		{
			m_on_value_changed = std::forward<Function>(func);
			return derived();
		}

		void handle_event(
			main::mouse_button_event const& mbe,
			main::window_ref window,
			main::ui_controller controller
		)
		{
			if(mbe.button == 0)
			{
				switch(mbe.action)
				{
					case main::mouse_button_action::press:
						m_value = derived().to_internal_value(mbe.where);
						m_on_value_changed(derived(), window, controller);
						m_state_current = state::handle_grabbed;
						break;

					case main::mouse_button_action::release:
						m_state_current = state::released;
						break;
				}
			}
			else
			if(mbe.action == main::mouse_button_action::release)
			{ m_state_current = state::released; }
		}

		void handle_event(
			main::keyboard_button_event const& event,
			main::window_ref window,
			main::ui_controller controller
		)
		{
			// TODO: Add "gears" to make speed variable
			auto const dx = 1.0f/64.0f;
			switch(to_builtin_command_id(event))
			{
				case main::builtin_command_id::step_left:
				case main::builtin_command_id::step_down:
					m_value = internal_value_type{m_value - dx, clamp_tag{}};
					m_on_value_changed(derived(), window, controller);
					break;

				case main::builtin_command_id::step_right:
				case main::builtin_command_id::step_up:
					m_value = internal_value_type{m_value + dx, clamp_tag{}};
					m_on_value_changed(derived(), window, controller);
					break;

				case main::builtin_command_id::go_to_begin:
					m_value = internal_value_type{0.0f, clamp_tag{}};
					m_on_value_changed(derived(), window, controller);
					break;

				case main::builtin_command_id::go_to_end:
					m_value = internal_value_type{1.0f, clamp_tag{}};
					m_on_value_changed(derived(), window, controller);
					break;

				default:

					break;
			}
		}

		auto& derived() &
		{ return static_cast<Derived&>(*this); }

		internal_value_type internal_value() const
		{ return m_value; }

		float to_value(float x) const
		{ return m_value_map.get().get_vtable().to_value(m_value_map.get().get_pointer(), x); }

		float from_value(float x) const
		{ return m_value_map.get().get_vtable().from_value(m_value_map.get().get_pointer(), x); }

	private:
		using user_interaction_handler = main::widget_user_interaction_handler<Derived>;
		user_interaction_handler m_on_value_changed{no_operation_tag{}};
		internal_value_type m_value;
		state m_state_current = state::released;

		type_erased_value_map m_value_map{
			std::in_place_type_t<terraformer::value_maps::affine_value_map>{}, 0.0f, 1.0f
		};
	};
}

#endif
