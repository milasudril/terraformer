//@	{"target":{"name":"widget_rendering_result.test"}}

#include "./widget_rendering_result.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	template<int n>
	struct dummy_texture
	{
		terraformer::span_2d<terraformer::rgba_pixel const> pixels{};

		void upload(terraformer::span_2d<terraformer::rgba_pixel const> new_pixels)
		{ pixels = new_pixels; }
	};

	struct dummy_result
	{
		using texture_type = dummy_texture<0>;

		auto set_widget_background(texture_type const* val, std::array<terraformer::rgba_pixel, 4> const& tints)
		{
			widget_background = val;
			widget_background_tints = tints;
			return terraformer::ui::main::set_texture_result::success;
		}

		auto set_bg_layer_mask(texture_type const* val)
		{
			bg_layer_mask = val;
			return terraformer::ui::main::set_texture_result::success;
		}

		auto set_selection_background(texture_type const* val, std::array<terraformer::rgba_pixel, 4> const& tints)
		{
			widget_background = val;
			widget_background_tints = tints;
			return terraformer::ui::main::set_texture_result::success;
		}

		auto set_widget_foreground(
			texture_type const* val,
			std::array<terraformer::rgba_pixel, 4> const& tints,
			terraformer::displacement offset
		)
		{
			widget_foreground = val;
			widget_foreground_tints = tints;
			widget_foreground_offset = offset;
			return terraformer::ui::main::set_texture_result::success;
		}

		auto set_input_marker(
			texture_type const*,
			std::array<terraformer::rgba_pixel, 4> const&,
			terraformer::displacement
		)
		{
			// TODO: Assign values
			return terraformer::ui::main::set_texture_result::success;
		}

		auto set_frame(texture_type const* val, std::array<terraformer::rgba_pixel, 4> const& tints)
		{
			frame = val;
			frame_tints = tints;
			return terraformer::ui::main::set_texture_result::success;
		}

		texture_type const* widget_background;
		texture_type const* bg_layer_mask;
		texture_type const* selection_background;
		texture_type const* widget_foreground;
		texture_type const* frame;
		std::array<terraformer::rgba_pixel, 4> widget_background_tints;
		std::array<terraformer::rgba_pixel, 4> selection_background_tints;
		std::array<terraformer::rgba_pixel, 4> widget_foreground_tints;
		std::array<terraformer::rgba_pixel, 4> frame_tints;
		terraformer::displacement widget_foreground_offset;
	};
}

TESTCASE(terraformer_ui_main_widget_rendering_result_create_texture)
{
	dummy_result res;
	terraformer::ui::main::widget_rendering_result result{std::ref(res)};

	auto texture = result.create_texture();
	std::array<terraformer::rgba_pixel, 2> pixels{};
	texture.upload(terraformer::span_2d<terraformer::rgba_pixel const>{1, 2, pixels.data()});
	auto real_texture = texture.get_if<dummy_texture<0>>();
	REQUIRE_NE(real_texture, nullptr);
	EXPECT_EQ(real_texture->pixels.width(), 1);
	EXPECT_EQ(real_texture->pixels.height(), 2);
	EXPECT_EQ(real_texture->pixels.data(), pixels.data());
}

TESTCASE(terraformer_ui_main_widget_rendering_result_set_values)
{
	// TODO: Test other fields

	dummy_result res{};
	terraformer::ui::main::widget_rendering_result result{std::ref(res)};

	auto bg = result.create_texture();
	auto fg = result.create_texture();
	std::array<terraformer::rgba_pixel, 4> bg_tints{
		terraformer::rgba_pixel{1.0f, 0.0f, 0.0f, 0.0f},
		terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 0.0f},
		terraformer::rgba_pixel{0.0f, 0.0f, 1.0f, 0.0f},
		terraformer::rgba_pixel{0.0f, 0.0f, 0.0f, 1.0f}
	};
	std::array<terraformer::rgba_pixel, 4> fg_tints{
		terraformer::rgba_pixel{0.0f, 1.0f, 1.0f, 1.0f},
		terraformer::rgba_pixel{1.0f, 0.0f, 1.0f, 1.0f},
		terraformer::rgba_pixel{1.0f, 1.0f, 0.0f, 1.0f},
		terraformer::rgba_pixel{1.0f, 1.0f, 1.0f, 0.0f}
	};

	result.set_widget_background(bg.get(), bg_tints);
	result.set_widget_foreground(fg.get(), fg_tints, terraformer::displacement{1.0f, 2.0f, 3.0f});

	REQUIRE_NE(bg.get_if<dummy_texture<0>>(), nullptr);
	REQUIRE_NE(fg.get_if<dummy_texture<0>>(), nullptr);

	EXPECT_EQ(res.widget_background, bg.get_if<dummy_texture<0>>());
	EXPECT_EQ(res.widget_foreground, fg.get_if<dummy_texture<0>>());
	EXPECT_EQ(res.widget_background_tints, bg_tints);
	EXPECT_EQ(res.widget_foreground_tints, fg_tints);
	EXPECT_EQ(res.widget_foreground_offset, (terraformer::displacement{1.0f, 2.0f, 3.0f}));

	dummy_texture<1> wrong_texture;
	result.set_widget_background(terraformer::ui::main::generic_texture_pointer_const{&wrong_texture}, bg_tints);
	EXPECT_EQ(res.widget_background, nullptr);
	result.set_widget_foreground(terraformer::ui::main::generic_texture_pointer_const{&wrong_texture}, fg_tints, terraformer::displacement{});
	EXPECT_EQ(res.widget_foreground, nullptr);
}

