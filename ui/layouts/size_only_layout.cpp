//@	{"target":{"name":"size_only_layout.o"}}

#include "./size_only_layout.hpp"

terraformer::scaling
terraformer::ui::layouts::size_only_layout::update_widget_locations(
	main::widget_collection_ref const& widgets
) const
{
	auto const widget_geometries = widgets.widget_geometries();
	float width = 0.0f;
	float height = 0.0f;

	for(auto k : widgets.element_indices())
	{
		width = std::max(
			  widget_geometries[k].where[0] + widget_geometries[k].size[0],
				width
		);

		height = std::max(
		  widget_geometries[k].where[1] + widget_geometries[k].size[1],
			height
		);
	}

	return scaling{
		width,
		height,
		1.0f,
	};
}