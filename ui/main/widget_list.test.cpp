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
	struct dummy_widget_no_default_visibility
	{
		size_t cursor_position_count{0};
		size_t mbe_count{0};
		size_t size_count{0};
		
		void render_to(dummy_surface& surface) const
		{ ++surface.update_count; }
		
		bool handle_event(terraformer::ui::wsapi::cursor_position)
		{ 
			++cursor_position_count;
			return true; 
		}
		
		bool handle_event(terraformer::ui::wsapi::mouse_button_event const&)
		{ 
			++mbe_count;
			return true;
			
		}

		auto handle_event(terraformer::ui::wsapi::fb_size size)
		{
			++size_count;
			return size;
		}
	};
	
	template<size_t N>
	struct dummy_widget:dummy_widget_no_default_visibility<N>
	{		
		static constexpr auto default_visibility = terraformer::ui::main::widget_visibility::hidden;
	};
}

TESTCASE(terraformer_ui_main_widget_list_append_stuff)
{
	terraformer::ui::main::widget_list<dummy_surface> widgets;
	EXPECT_EQ(std::size(widgets).get(), 0);
	
	dummy_widget<0> widget_0;
	dummy_widget<1> widget_1;
	dummy_widget<2> widget_2;
	dummy_widget_no_default_visibility<3> widget_3;
	
	widgets.append(std::ref(widget_0))
		.append(std::ref(widget_1))
		.append(std::ref(widget_2), terraformer::ui::main::widget_visibility::skipped)
		.append(std::ref(widget_3), terraformer::ui::main::widget_visibility::visible);
		
	EXPECT_EQ(std::size(widgets).get(), 4);
	
	// Check pointers and visibilities
	{
		auto const widget_ptrs = widgets.widget_pointers();
		auto const visibilities = widgets.widget_visibilities();
		for(auto k = widgets.first_element_index();
			k != std::size(widgets);
			++k
		)
		{
			switch(k.get())
			{
				case 0:
					EXPECT_EQ(widget_ptrs[k], &widget_0);
					EXPECT_EQ(visibilities[k], dummy_widget<0>::default_visibility);
					break;
				case 1:
					EXPECT_EQ(widget_ptrs[k], &widget_1);
					EXPECT_EQ(visibilities[k], dummy_widget<0>::default_visibility);
					break;
				case 2:
					EXPECT_EQ(widget_ptrs[k], &widget_2);
					EXPECT_EQ(visibilities[k], terraformer::ui::main::widget_visibility::skipped);
					break;
				case 3:
					EXPECT_EQ(widget_ptrs[k], &widget_3);
					EXPECT_EQ(visibilities[k], terraformer::ui::main::widget_visibility::visible);
					break;
			}
		}
	}
	
	// Call render_to
	{
		auto const widget_ptrs = widgets.widget_pointers();
		auto const render_callbacks = widgets.render_callbacks();
		dummy_surface surface{};
		
		for(auto k =  widgets.first_element_index();
			k != std::size(widgets);
			++k
		)
		{
			render_callbacks[k](widget_ptrs[k], surface);
			EXPECT_EQ(surface.update_count, k.get() + 1);
			
			EXPECT_EQ(widget_0.cursor_position_count, 0);
			EXPECT_EQ(widget_0.mbe_count, 0);
			EXPECT_EQ(widget_0.size_count, 0);
			
			EXPECT_EQ(widget_1.cursor_position_count, 0);
			EXPECT_EQ(widget_1.mbe_count, 0);
			EXPECT_EQ(widget_1.size_count, 0);
			
			EXPECT_EQ(widget_2.cursor_position_count, 0);
			EXPECT_EQ(widget_2.mbe_count, 0);
			EXPECT_EQ(widget_2.size_count, 0);

			EXPECT_EQ(widget_3.cursor_position_count, 0);
			EXPECT_EQ(widget_3.mbe_count, 0);
			EXPECT_EQ(widget_3.size_count, 0);
		}
	}
}
