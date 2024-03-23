//@	{
//@		"target":{"name":"terraformer-gui.o"}
//@	}

#include "./wsapi/gl_viewport.hpp"

namespace
{
	bool do_main_iteration(terraformer::gl_viewport& viewport)
	{
		viewport.swap_buffers();
		return false;
	}
}

int main(int, char**)
{
	auto& gui_ctxt = terraformer::glfw_context::get();
	terraformer::gl_viewport mainwin{gui_ctxt, 800, 500, "Terraformer"};
	gui_ctxt.read_events(do_main_iteration, std::ref(mainwin));
}
