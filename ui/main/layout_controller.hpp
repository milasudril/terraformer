#ifndef TERRAFORMER_UI_MAIN_LAYOUT_CONTROLLER_HPP
#define TERRAFORMER_UI_MAIN_LAYOUT_CONTROLLER_HPP

namespace terraformer::ui::main
{
	template<class T>
	concept layout_controller = requires (T& obj)
	{
		{obj.update_widget_geometries()}->std::same_as<void>;
	};
}

#endif