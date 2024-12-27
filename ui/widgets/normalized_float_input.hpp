//@	{"dependencies_extra":[{"ref":"./slider.o", "rel":"implementation"}]}

#ifndef TERRAFORMER_UI_WIDGETS_NORMALIZED_FLOAT_INPUT_HPP
#define TERRAFORMER_UI_WIDGETS_NORMALIZED_FLOAT_INPUT_HPP

#include "ui/main/widget.hpp"
#include "lib/common/move_only_function.hpp"

namespace terraformer::ui::widgets
{
	template<class Derived>
	class normalized_float_input:public main::widget_with_default_actions
	{
	public:
		using widget_with_default_actions::handle_event;

		enum class state{released, handle_grabbed};

		void handle_event(
			main::cursor_motion_event const& cme,
			main::window_ref window,
			main::ui_controller controller
		) 
		{
			if(m_state_current == state::handle_grabbed)
			{ 
				value(derived().to_value(cme.where)); 
				m_on_value_changed(derived(), window, controller);
			}
		}

		void value(float new_val)
		{ m_value = std::clamp(new_val, 0.0f, 1.0f); }
		
		float value() const
		{ return m_value; }
		
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
						value(derived().to_value(mbe.where));
						m_on_value_changed(derived(), window, controller);
						m_state_current = state::handle_grabbed;
						break;

					case main::mouse_button_action::release:
						m_state_current = state::released;
						break;
				}
			}
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
					value(m_value - dx); 
					m_on_value_changed(derived(), window, controller);
					break;

				case main::builtin_command_id::step_right:
				case main::builtin_command_id::step_up:
					value(m_value + dx); 
					m_on_value_changed(derived(), window, controller);
					break;
				default:

					break;
			}
		}
		
		auto& derived() &
		{ return static_cast<Derived&>(*this); }

	private:
		using user_interaction_handler = main::widget_user_interaction_handler<Derived>;
		user_interaction_handler m_on_value_changed{no_operation_tag{}};
		float m_value = 0.0f;
		state m_state_current = state::released;
	};
}

#endif
