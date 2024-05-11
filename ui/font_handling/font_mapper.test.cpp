//@	 {"target": { "name":"font_mapper.test"}}

#include "./font_mapper.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_ui_font_handling_font_mapper_move)
{
	terraformer::ui::font_handling::font_mapper mapper;
	REQUIRE_EQ(mapper.valid(), true);
	auto other = std::move(mapper);
	EXPECT_EQ(mapper.valid(), false);
	EXPECT_EQ(other.valid(), true);
}

TESTCASE(terraformer_ui_font_handling_font_mapper_get_path)
{
	terraformer::ui::font_handling::font_mapper mapper;

	auto result_a = mapper.get_path("sans-serif");
	EXPECT_NE(std::size(result_a.string()), 0);
	auto stat_a = status(result_a);
	EXPECT_NE(static_cast<int>(stat_a.type()), static_cast<int>(std::filesystem::file_type::not_found));

	auto result_b = mapper.get_path("serif");
	EXPECT_NE(std::size(result_b.string()), 0);
	auto stat_b = status(result_b);
	EXPECT_NE(static_cast<int>(stat_b.type()), static_cast<int>(std::filesystem::file_type::not_found));

	EXPECT_NE(result_a, result_b);
}

TESTCASE(terraformer_ui_font_handling_font_mapper_get_path_failed)
{
	auto res = terraformer::ui::font_handling::font_mapper{}.get_path("non-existing font");
	EXPECT_NE(std::size(res.string()), 0);
	auto stat = status(res);
	EXPECT_NE(static_cast<int>(stat.type()), static_cast<int>(std::filesystem::file_type::not_found));
}