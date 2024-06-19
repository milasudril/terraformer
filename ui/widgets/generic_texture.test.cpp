//@	{"target":{"name":"generic_texture.test"}}

#include "./generic_texture.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_generic_texture_default_state)
{
	terraformer::ui::widgets::generic_shared_texture texture;
	EXPECT_EQ(texture.get_if<int>(), nullptr);
	EXPECT_EQ(texture.get(), false);
	EXPECT_EQ(texture.get_const(), false);
	EXPECT_EQ(texture.use_count(), 0);
	EXPECT_EQ(texture, false);
	EXPECT_EQ(texture.object_id(), 0);
}

namespace
{
	template<int n>
	struct dummy_texture
	{
		terraformer::span_2d<terraformer::rgba_pixel const> pixels{};

		void upload(terraformer::span_2d<terraformer::rgba_pixel const> new_pixels)
		{ pixels = new_pixels; }
	};
}

TESTCASE(terraformer_generic_texture_create_and_reset)
{
	terraformer::ui::widgets::generic_shared_texture texture{std::type_identity<dummy_texture<0>>{}};
	EXPECT_EQ(texture.get_if<dummy_texture<1>>(), nullptr);
	EXPECT_EQ(texture.get(), true);
	EXPECT_EQ(texture.get_const(), true);
	EXPECT_EQ(texture.use_count(), 1);
	EXPECT_EQ(texture, true);
	EXPECT_EQ(texture.object_id(), reinterpret_cast<intptr_t>(texture.get().pointer()));

	auto const value = texture.get_if<dummy_texture<0> const>();
	REQUIRE_NE(value, nullptr);
	dummy_texture<0> const* value_other = texture.get();
	EXPECT_EQ(value, value_other);
	EXPECT_EQ(value->pixels.width(), 0);
	EXPECT_EQ(value->pixels.height(), 0);
	EXPECT_EQ(value->pixels.data(), nullptr);
	EXPECT_EQ(texture.get_if<dummy_texture<1>>(), nullptr);

	std::array<terraformer::rgba_pixel, 2> pixels{};
	texture.upload(terraformer::span_2d<terraformer::rgba_pixel const>{1, 2, pixels.data()});

	EXPECT_EQ(value->pixels.width(), 1);
	EXPECT_EQ(value->pixels.height(), 2);
	EXPECT_EQ(value->pixels.data(), pixels.data());

	texture.reset();

	EXPECT_EQ(texture.get_if<dummy_texture<0>>(), nullptr);
	EXPECT_EQ(texture.get(), false);
	EXPECT_EQ(texture.get_const(), false);
	EXPECT_EQ(texture.use_count(), 0);
	EXPECT_EQ(texture, false);
	EXPECT_EQ(texture.object_id(), 0);
}

TESTCASE(terraformer_generic_texture_create_and_assign_other)
{
	terraformer::ui::widgets::generic_shared_texture texture{std::type_identity<dummy_texture<0>>{}};
	EXPECT_EQ(texture.get_if<dummy_texture<1>>(), nullptr);
	EXPECT_EQ(texture.get(), true);
	EXPECT_EQ(texture.get_const(), true);
	EXPECT_EQ(texture.use_count(), 1);
	EXPECT_EQ(texture, true);
	EXPECT_EQ(texture.object_id(), reinterpret_cast<intptr_t>(texture.get().pointer()));

	auto const value = texture.get_if<dummy_texture<0> const>();
	REQUIRE_NE(value, nullptr);
	dummy_texture<0> const* value_other = texture.get();
	EXPECT_EQ(value, value_other);
	EXPECT_EQ(value->pixels.width(), 0);
	EXPECT_EQ(value->pixels.height(), 0);
	EXPECT_EQ(value->pixels.data(), nullptr);
	EXPECT_EQ(texture.get_if<dummy_texture<1>>(), nullptr);

	std::array<terraformer::rgba_pixel, 2> pixels{};
	texture.upload(terraformer::span_2d<terraformer::rgba_pixel const>{1, 2, pixels.data()});

	EXPECT_EQ(value->pixels.width(), 1);
	EXPECT_EQ(value->pixels.height(), 2);
	EXPECT_EQ(value->pixels.data(), pixels.data());

	texture = dummy_texture<1>{};

	EXPECT_EQ(texture.get_if<dummy_texture<0>>(), nullptr);
	EXPECT_NE(texture.get_if<dummy_texture<1>>(), nullptr);
}