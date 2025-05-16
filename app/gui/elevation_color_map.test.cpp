//@	{"target":{"name":"elevation_color_map.test"}}

#include "./elevation_color_map.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_elevation_color_map_values)
{
	for(auto item : terraformer::elevation_colors)
	{
		printf("%.8g %.8g %.8g %.8g\n", item.red(), item.green(), item.blue(), item.alpha());
	}
}