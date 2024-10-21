//@	{"target":{"name":"window_ref.test"}}

#include "./window_ref.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct window_handle;

	void set_title(window_handle& obj, char const* str);
}

namespace terraformer::ui::main
{
	template<>
	struct window_traits<window_handle>
	{
		static void set_title(window_handle& obj, char const* str)
		{ ::set_title(obj, str); }
	};
};

namespace
{
	struct window_handle
	{
		std::string title;
	};

	void set_title(window_handle& obj, char const* str)
	{
		obj.title = str;
	}
}



TESTCASE(terraformer_ui_window_ref_test_callbacks)
{
	window_handle actual_window;
	terraformer::ui::main::window_ref ref{actual_window};

	ref.set_title("This is a test");
	EXPECT_EQ(actual_window.title, "This is a test");
}