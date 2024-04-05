//@	{"target":{"name": "widget_list.test"}}

#include "./widget_list.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct dummy_surface
	{};

	template<size_t N>
	struct dummy_widget
	{
		size_t cursor_position_count{0};
		size_t mbe_count{0};
		size_t size_count{0};
		size_t render_count{0};

		void render()
		{ ++render_count; }

		dummy_surface drawing_surface() const
		{ return dummy_surface{}; }

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
}

TESTCASE(terraformer_ui_main_widget_list_append_stuff)
{
	terraformer::ui::main::widget_list<dummy_surface> widgets;
	EXPECT_EQ(std::size(widgets).get(), 0);

	dummy_widget<0> widget_0;
	dummy_widget<1> widget_1;
	dummy_widget<2> widget_2;
	dummy_widget<3> widget_3;

	std::array<terraformer::ui::main::widget_geometry, 4> const initial_geometries{
		terraformer::ui::main::widget_geometry{1, 2, 3, 4},
		terraformer::ui::main::widget_geometry{5, 6, 7, 8},
		terraformer::ui::main::widget_geometry{9, 10, 11, 12},
		terraformer::ui::main::widget_geometry{13, 14, 15, 16}
	};

	std::array<terraformer::ui::main::widget_visibility, 4> const initial_visibilities{
		terraformer::ui::main::widget_visibility::visible,
		terraformer::ui::main::widget_visibility::visible,
		terraformer::ui::main::widget_visibility::not_rendered,
		terraformer::ui::main::widget_visibility::collapsed,
	};

	widgets.append(std::ref(widget_0), initial_geometries[0])
		.append(std::ref(widget_1), initial_geometries[1], initial_visibilities[1])
		.append(std::ref(widget_2), initial_geometries[2], initial_visibilities[2])
		.append(std::ref(widget_3), initial_geometries[3], initial_visibilities[3]);

	EXPECT_EQ(std::size(widgets).get(), 4);

	// Check properties
	{
		auto const widget_ptrs = widgets.widget_pointers();
		auto const visibilities = widgets.widget_visibilities();
		auto const geometries = widgets.widget_geometries();

		for(auto k = widgets.first_element_index();
			k != std::size(widgets);
			++k
		)
		{
			EXPECT_EQ(visibilities[k], initial_visibilities[k.get()]);
			EXPECT_EQ(geometries[k], initial_geometries[k.get()]);

			switch(k.get())
			{
				case 0:
					EXPECT_EQ(widget_ptrs[k], &widget_0);
					break;
				case 1:
					EXPECT_EQ(widget_ptrs[k], &widget_1);
					break;
				case 2:
					EXPECT_EQ(widget_ptrs[k], &widget_2);
					break;
				case 3:
					EXPECT_EQ(widget_ptrs[k], &widget_3);
					break;
			}
		}
	}

	// Call render
	{
		auto const widget_ptrs = widgets.widget_pointers();
		auto const render_callbacks = widgets.render_callbacks();

		for(auto k =  widgets.first_element_index();
			k != std::size(widgets);
			++k
		)
		{
			render_callbacks[k](widget_ptrs[k]);

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

	// Call handle_event cursor_position
	{
		auto const widget_ptrs = widgets.widget_pointers();
		auto const cursor_position_callbacks = widgets.cursor_position_callbacks();

		for(auto k =  widgets.first_element_index();
			k != std::size(widgets);
			++k
		)
		{
			cursor_position_callbacks[k](widget_ptrs[k], terraformer::ui::wsapi::cursor_position{});

			switch(k.get())
			{
				case 0:
					EXPECT_EQ(widget_0.cursor_position_count, 1);
					EXPECT_EQ(widget_1.cursor_position_count, 0);
					EXPECT_EQ(widget_2.cursor_position_count, 0);
					EXPECT_EQ(widget_3.cursor_position_count, 0);
					break;
				case 1:
					EXPECT_EQ(widget_0.cursor_position_count, 1);
					EXPECT_EQ(widget_1.cursor_position_count, 1);
					EXPECT_EQ(widget_2.cursor_position_count, 0);
					EXPECT_EQ(widget_3.cursor_position_count, 0);
					break;
				case 2:
					EXPECT_EQ(widget_0.cursor_position_count, 1);
					EXPECT_EQ(widget_1.cursor_position_count, 1);
					EXPECT_EQ(widget_2.cursor_position_count, 1);
					EXPECT_EQ(widget_3.cursor_position_count, 0);
					break;
				case 3:
					EXPECT_EQ(widget_0.cursor_position_count, 1);
					EXPECT_EQ(widget_1.cursor_position_count, 1);
					EXPECT_EQ(widget_2.cursor_position_count, 1);
					EXPECT_EQ(widget_3.cursor_position_count, 1);
					break;
			}

			EXPECT_EQ(widget_0.mbe_count, 0);
			EXPECT_EQ(widget_0.size_count, 0);
			EXPECT_EQ(widget_1.mbe_count, 0);
			EXPECT_EQ(widget_1.size_count, 0);
			EXPECT_EQ(widget_2.mbe_count, 0);
			EXPECT_EQ(widget_2.size_count, 0);
			EXPECT_EQ(widget_3.mbe_count, 0);
			EXPECT_EQ(widget_3.size_count, 0);
		}
	}


	// Call handle_event mouse_button_event
	{
		auto const widget_ptrs = widgets.widget_pointers();
		auto const mouse_button_callbacks = widgets.mouse_button_callbacks();

		for(auto k =  widgets.first_element_index();
			k != std::size(widgets);
			++k
		)
		{
			mouse_button_callbacks[k](widget_ptrs[k], terraformer::ui::wsapi::mouse_button_event{});

			switch(k.get())
			{
				case 0:
					EXPECT_EQ(widget_0.mbe_count, 1);
					EXPECT_EQ(widget_1.mbe_count, 0);
					EXPECT_EQ(widget_2.mbe_count, 0);
					EXPECT_EQ(widget_3.mbe_count, 0);
					break;
				case 1:
					EXPECT_EQ(widget_0.mbe_count, 1);
					EXPECT_EQ(widget_1.mbe_count, 1);
					EXPECT_EQ(widget_2.mbe_count, 0);
					EXPECT_EQ(widget_3.mbe_count, 0);
					break;
				case 2:
					EXPECT_EQ(widget_0.mbe_count, 1);
					EXPECT_EQ(widget_1.mbe_count, 1);
					EXPECT_EQ(widget_2.mbe_count, 1);
					EXPECT_EQ(widget_3.mbe_count, 0);
					break;
				case 3:
					EXPECT_EQ(widget_0.mbe_count, 1);
					EXPECT_EQ(widget_1.mbe_count, 1);
					EXPECT_EQ(widget_2.mbe_count, 1);
					EXPECT_EQ(widget_3.mbe_count, 1);
					break;
			}

			EXPECT_EQ(widget_0.cursor_position_count, 1);
			EXPECT_EQ(widget_1.cursor_position_count, 1);
			EXPECT_EQ(widget_2.cursor_position_count, 1);
			EXPECT_EQ(widget_3.cursor_position_count, 1);

			EXPECT_EQ(widget_0.size_count, 0);
			EXPECT_EQ(widget_1.size_count, 0);
			EXPECT_EQ(widget_2.size_count, 0);
			EXPECT_EQ(widget_3.size_count, 0);
		}
	}

	// Call handle_event fb_size
	{
		auto const widget_ptrs = widgets.widget_pointers();
		auto const size_callbacks = widgets.size_callbacks();

		for(auto k =  widgets.first_element_index();
			k != std::size(widgets);
			++k
		)
		{
			size_callbacks[k](widget_ptrs[k], terraformer::ui::wsapi::fb_size{});

			switch(k.get())
			{
				case 0:
					EXPECT_EQ(widget_0.size_count, 1);
					EXPECT_EQ(widget_1.size_count, 0);
					EXPECT_EQ(widget_2.size_count, 0);
					EXPECT_EQ(widget_3.size_count, 0);
					break;
				case 1:
					EXPECT_EQ(widget_0.size_count, 1);
					EXPECT_EQ(widget_1.size_count, 1);
					EXPECT_EQ(widget_2.size_count, 0);
					EXPECT_EQ(widget_3.size_count, 0);
					break;
				case 2:
					EXPECT_EQ(widget_0.size_count, 1);
					EXPECT_EQ(widget_1.size_count, 1);
					EXPECT_EQ(widget_2.size_count, 1);
					EXPECT_EQ(widget_3.size_count, 0);
					break;
				case 3:
					EXPECT_EQ(widget_0.size_count, 1);
					EXPECT_EQ(widget_1.size_count, 1);
					EXPECT_EQ(widget_2.size_count, 1);
					EXPECT_EQ(widget_3.size_count, 1);
					break;
			}

			EXPECT_EQ(widget_0.cursor_position_count, 1);
			EXPECT_EQ(widget_1.cursor_position_count, 1);
			EXPECT_EQ(widget_2.cursor_position_count, 1);
			EXPECT_EQ(widget_3.cursor_position_count, 1);

			EXPECT_EQ(widget_0.mbe_count, 1);
			EXPECT_EQ(widget_1.mbe_count, 1);
			EXPECT_EQ(widget_2.mbe_count, 1);
			EXPECT_EQ(widget_3.mbe_count, 1);
		}
	}
}

TESTCASE(terraformer_ui_main_widget_list_render_widgets)
{
	terraformer::ui::main::widget_list<dummy_surface> widgets;

	dummy_widget<0> widget_0;
	dummy_widget<1> widget_1;
	dummy_widget<2> widget_2;
	dummy_widget<3> widget_3;

	std::array<terraformer::ui::main::widget_geometry, 4> const initial_geometries{
		terraformer::ui::main::widget_geometry{1, 2, 3, 4},
		terraformer::ui::main::widget_geometry{5, 6, 7, 8},
		terraformer::ui::main::widget_geometry{9, 10, 11, 12},
		terraformer::ui::main::widget_geometry{13, 14, 15, 16}
	};

	std::array<terraformer::ui::main::widget_visibility, 4> const initial_visibilities{
		terraformer::ui::main::widget_visibility::visible,
		terraformer::ui::main::widget_visibility::visible,
		terraformer::ui::main::widget_visibility::not_rendered,
		terraformer::ui::main::widget_visibility::collapsed,
	};

	widgets.append(std::ref(widget_0), initial_geometries[0])
		.append(std::ref(widget_1), initial_geometries[1], initial_visibilities[1])
		.append(std::ref(widget_2), initial_geometries[2], initial_visibilities[2])
		.append(std::ref(widget_3), initial_geometries[3], initial_visibilities[3]);

	render_widgets(widgets);

	auto const widget_visibilities = widgets.widget_visibilities();
	for(auto k =  widgets.first_element_index();
		k != std::size(widgets);
		++k
	)
	{
		if(widget_visibilities[k] == terraformer::ui::main::widget_visibility::visible)
		{
//			EXPECT_EQ(widget_surfaces[k].update_count, 1);
		}
		else
		{
			//EXPECT_EQ(widget_surfaces[k].update_count, 0);

		}
	}
}

namespace
{
	struct dummy_renderer
	{
		void render_surface(dummy_surface const&, terraformer::ui::main::widget_geometry const&)
		{ ++callcount; }

		size_t callcount{0};
	};
}

TESTCASE(terraformer_ui_main_widget_list_show_widgets)
{
	terraformer::ui::main::widget_list<dummy_surface> widgets;

	dummy_widget<0> widget_0;
	dummy_widget<1> widget_1;
	dummy_widget<2> widget_2;
	dummy_widget<3> widget_3;

	std::array<terraformer::ui::main::widget_geometry, 4> const initial_geometries{
		terraformer::ui::main::widget_geometry{1, 2, 3, 4},
		terraformer::ui::main::widget_geometry{5, 6, 7, 8},
		terraformer::ui::main::widget_geometry{9, 10, 11, 12},
		terraformer::ui::main::widget_geometry{13, 14, 15, 16}
	};

	std::array<terraformer::ui::main::widget_visibility, 4> const initial_visibilities{
		terraformer::ui::main::widget_visibility::visible,
		terraformer::ui::main::widget_visibility::visible,
		terraformer::ui::main::widget_visibility::not_rendered,
		terraformer::ui::main::widget_visibility::collapsed,
	};

	widgets.append(std::ref(widget_0), initial_geometries[0])
		.append(std::ref(widget_1), initial_geometries[1], initial_visibilities[1])
		.append(std::ref(widget_2), initial_geometries[2], initial_visibilities[2])
		.append(std::ref(widget_3), initial_geometries[3], initial_visibilities[3]);

	dummy_renderer renderer{};
	show_widgets(renderer, widgets);
	EXPECT_EQ(renderer.callcount, 2);
}

TESTCASE(terraformer_ui_main_widget_list_find_widget_by_cursor_position)
{
	terraformer::ui::main::widget_list<dummy_surface> widgets;

	dummy_widget<0> widget_0;
	dummy_widget<1> widget_1;
	dummy_widget<2> widget_2;
	dummy_widget<3> widget_3;

	std::array<terraformer::ui::main::widget_geometry, 4> const initial_geometries{
		terraformer::ui::main::widget_geometry{1, 2, 3, 4},
		terraformer::ui::main::widget_geometry{5, 6, 7, 8},
		terraformer::ui::main::widget_geometry{9, 10, 11, 12},
		terraformer::ui::main::widget_geometry{13, 14, 15, 16}
	};

	std::array<terraformer::ui::main::widget_visibility, 4> const initial_visibilities{
		terraformer::ui::main::widget_visibility::visible,
		terraformer::ui::main::widget_visibility::visible,
		terraformer::ui::main::widget_visibility::not_rendered,
		terraformer::ui::main::widget_visibility::collapsed,
	};

	widgets.append(std::ref(widget_0), initial_geometries[0])
		.append(std::ref(widget_1), initial_geometries[1], initial_visibilities[1])
		.append(std::ref(widget_2), initial_geometries[2], initial_visibilities[2])
		.append(std::ref(widget_3), initial_geometries[3], initial_visibilities[3]);

	auto const i0 = find(terraformer::ui::wsapi::cursor_position{2.0, 3.0}, widgets);
	auto const i1 = find(terraformer::ui::wsapi::cursor_position{6.0, 7.0}, widgets);
	auto const i2 = find(terraformer::ui::wsapi::cursor_position{19.0, 11.0}, widgets);
	auto const i3 = find(terraformer::ui::wsapi::cursor_position{27.0, 15.0}, widgets);
	auto const i_end = find(terraformer::ui::wsapi::cursor_position{28.0, 15.0}, widgets);

	EXPECT_EQ(i0.get(), 0);
	EXPECT_EQ(i1.get(), 1);
	EXPECT_EQ(i2.get(), 2);
	EXPECT_EQ(i3.get(), 3);
	EXPECT_EQ(i_end, widgets.npos);
}
