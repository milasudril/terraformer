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

		void set_background(texture_type const* val)
		{ background = val; }

		void set_foreground(texture_type const* val)
		{ foreground = val; }

		void set_background_tints(std::array<terraformer::rgba_pixel, 4> const& vals)
		{ background_tints = vals;}

		void set_foreground_tints(std::array<terraformer::rgba_pixel, 4> const& vals)
		{ foreground_tints = vals; }

		texture_type const* background;
		texture_type const* foreground;
		std::array<terraformer::rgba_pixel, 4> background_tints;
		std::array<terraformer::rgba_pixel, 4> foreground_tints;
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

	result.set_background(bg.get());
	result.set_foreground(fg.get());
	result.set_background_tints(bg_tints);
	result.set_foreground_tints(fg_tints);

	REQUIRE_NE(bg.get_if<dummy_texture<0>>(), nullptr);
	REQUIRE_NE(fg.get_if<dummy_texture<0>>(), nullptr);

	EXPECT_EQ(res.background, bg.get_if<dummy_texture<0>>());
	EXPECT_EQ(res.foreground, fg.get_if<dummy_texture<0>>());
	EXPECT_EQ(res.background_tints, bg_tints);
	EXPECT_EQ(res.foreground_tints, fg_tints);

	dummy_texture<1> wrong_texture;
	result.set_background(terraformer::ui::main::generic_texture_pointer_const{&wrong_texture});
	EXPECT_EQ(res.background, nullptr);
	result.set_foreground(terraformer::ui::main::generic_texture_pointer_const{&wrong_texture});
	EXPECT_EQ(res.foreground, nullptr);
}

