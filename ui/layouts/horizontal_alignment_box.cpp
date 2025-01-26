//@	{"target":{"name":"horizontal_alignment_box.o"}}

#include "./horizontal_alignment_box.hpp"

terraformer::scaling
terraformer::ui::layouts::horizontal_alignment_box::update_widget_locations(
	main::widget_collection_ref& widgets
) const
{
	auto const indices = widgets.element_indices();
	auto const geometries = widgets.widget_geometries();
	auto const width = m_width;
	for(auto k : indices)
	{
		auto const where = geometries[k].where[0];
		geometries[k].where[0] = 0.5f*width + where;
	}
	return terraformer::scaling{m_width, m_height, 0.0f};
}

void terraformer::ui::layouts::horizontal_alignment_box::minimize_cell_sizes(
	main::widget_collection_ref const& widgets
)
{
	auto max_width = 0.0f;
	auto height = 0.0f;
	auto const indices = widgets.element_indices();
	auto const geometries = widgets.widget_geometries();
	auto const sizes = widgets.sizes();
	for(auto k : indices)
	{
		auto const box = geometries[k];
		auto const r = 0.5f*sizes[k];
		auto const offset_to_origin = (location{1.0f, -1.0f, 0.0f} - box.origin).apply(r);
		auto const object_loc = box.where - offset_to_origin;
		height = std::max(height, -object_loc[1] + sizes[k][1]);
		max_width = std::max(
			sizes[k][0] +	2.0f* params.margin_x,
			max_width
		);
	}

	printf("%.8g\n", height);

	m_width = max_width;
	m_height = height;
}