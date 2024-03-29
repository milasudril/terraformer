//@	{"target":{"name": "widget_list.test"}}

#include "./widget_list.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct dummy_surface
	{
		size_t update_count{0};
	};
	
	template<size_t N>
	struct dummy_widget
	{
		static constexpr auto default_visibility = terraformer::ui::main::widget_visibility::hidden;
		
		void render_to(dummy_surface& surface) const
		{ ++surface.update_count; }
		
		bool handle_event(terraformer::ui::wsapi::cursor_position)
		{ return true; }
		
		bool handle_event(terraformer::ui::wsapi::mouse_button_event const&)
		{ return true; }
	
		auto handle_event(terraformer::ui::wsapi::fb_size size)
		{ return size; }
	};
	
	template<size_t N>
	struct dummy_widget_no_default_visibility
	{		
		void render_to(dummy_surface& surface) const
		{ ++surface.update_count; }
		
		bool handle_event(terraformer::ui::wsapi::cursor_position)
		{ return true; }
		
		bool handle_event(terraformer::ui::wsapi::mouse_button_event const&)
		{ return true; }
	
		auto handle_event(terraformer::ui::wsapi::fb_size size)
		{ return size; }
	};
}

TESTCASE(terraformer_ui_main_widget_list_append_stuff)
{
	terraformer::ui::main::widget_list<dummy_surface> widgets;
	EXPECT_EQ(std::size(widgets).get(), 0);
	
	dummy_widget<0> widget_0;
	dummy_widget<1> widget_1;
	dummy_widget<2> widget_2;
	
	widgets.append(std::ref(widget_0))
		.append(std::ref(widget_1))
		.append(std::ref(widget_2));
}
