//@	{"target":{"name":"window_ref.test"}}

#include "./window_ref.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct window_handle;

	void set_title(window_handle& obj, char const* str);
	void set_clipboard_string(window_handle& obj, char8_t const* str);
	std::u8string get_clipboard_string(window_handle const& obj);
}

namespace terraformer::ui::main
{
	template<>
	struct window_traits<window_handle>
	{
		static void set_title(window_handle& obj, char const* str)
		{ ::set_title(obj, str); }

		static void set_clipboard_string(window_handle& obj, char8_t const* str)
		{ ::set_clipboard_string(obj, str); }

		static auto get_clipboard_string(window_handle const& obj)
		{ return ::get_clipboard_string(obj); }
	};
};

namespace
{
	struct window_handle
	{
		std::string title;
		std::u8string clipboard;
	};

	void set_title(window_handle& obj, char const* str)
	{
		obj.title = str;
	}

	void set_clipboard_string(window_handle& obj, char8_t const* str)
	{
		obj.clipboard = str;
	}

	std::u8string get_clipboard_string(window_handle const& obj)
	{ return obj.clipboard; }
}



TESTCASE(terraformer_ui_window_ref_test_callbacks)
{
	window_handle actual_window;
	terraformer::ui::main::window_ref ref{actual_window};

	ref.set_title("This is a test");
	EXPECT_EQ(actual_window.title, "This is a test");
}