#ifndef TERRAFORMER_UI_WIDGETS_WIDGET_CANVAS_HPP
#define TERRAFORMER_UI_WIDGETS_WIDGET_CANVAS_HPP

#include "./widget_group.hpp"
#include "ui/layouts/size_only_layout.hpp"

namespace terraformer::ui::widgets
{
	using widget_canvas = widget_group<layouts::size_only_layout>;
}

#endif