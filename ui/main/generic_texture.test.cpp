//@	{"target":{"name":"generic_texture.test"}}

#include "./generic_texture.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_generic_texture_default_state)
{
	terraformer::ui::main::generic_shared_texture texture;
	EXPECT_EQ(texture.get_if<int>(), nullptr);
	EXPECT_EQ(texture.get_stored_any(), false);
	EXPECT_EQ(texture.get_stored_any_const(), false);
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
	terraformer::ui::main::generic_shared_texture texture{std::type_identity<dummy_texture<0>>{}};
	EXPECT_EQ(texture.get_if<dummy_texture<1>>(), nullptr);
	EXPECT_EQ(texture.get_stored_any(), true);
	EXPECT_EQ(texture.get_stored_any_const(), true);
	EXPECT_EQ(texture.use_count(), 1);
	EXPECT_EQ(texture, true);
	EXPECT_EQ(texture.object_id(), reinterpret_cast<intptr_t>(texture.get_stored_any().pointer()));

	auto const value = texture.get_if<dummy_texture<0> const>();
	REQUIRE_NE(value, nullptr);
	dummy_texture<0> const* value_other = texture.get_stored_any();
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
	EXPECT_EQ(texture.get_stored_any(), false);
	EXPECT_EQ(texture.get_stored_any_const(), false);
	EXPECT_EQ(texture.use_count(), 0);
	EXPECT_EQ(texture, false);
	EXPECT_EQ(texture.object_id(), 0);
}

TESTCASE(terraformer_generic_texture_create_and_assign_other)
{
	terraformer::ui::main::generic_shared_texture texture{std::type_identity<dummy_texture<0>>{}};
	EXPECT_EQ(texture.get_if<dummy_texture<1>>(), nullptr);
	EXPECT_EQ(texture.get_stored_any(), true);
	EXPECT_EQ(texture.get_stored_any_const(), true);
	EXPECT_EQ(texture.use_count(), 1);
	EXPECT_EQ(texture, true);
	EXPECT_EQ(texture.object_id(), reinterpret_cast<intptr_t>(texture.get_stored_any().pointer()));

	auto const value = texture.get_if<dummy_texture<0> const>();
	REQUIRE_NE(value, nullptr);
	dummy_texture<0> const* value_other = texture.get_stored_any();
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

TESTCASE(terraformer_generic_texture_pointer)
{
	dummy_texture<0> ref_to;
	terraformer::ui::main::generic_texture_pointer texture{&ref_to};
	EXPECT_EQ(texture.get_if<dummy_texture<1>>(), nullptr);
	EXPECT_EQ(texture.get_stored_any(), true);
	EXPECT_EQ(texture.get_stored_any_const(), true);
	EXPECT_EQ(texture, true);
	EXPECT_EQ(texture.object_id(), reinterpret_cast<intptr_t>(texture.get_stored_any().pointer()));

	auto const value = texture.get_if<dummy_texture<0> const>();
	EXPECT_EQ(value, &ref_to);
	dummy_texture<0> const* value_other = texture.get_stored_any();
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

	dummy_texture<1> other_obj;
	texture = &other_obj;

	EXPECT_EQ(texture.get_if<dummy_texture<0>>(), nullptr);
	EXPECT_EQ(texture.get_if<dummy_texture<1>>(), &other_obj);
}

TESTCASE(terraformer_generic_texture_get_pointer)
{
	terraformer::ui::main::generic_shared_texture texture{std::type_identity<dummy_texture<0>>{}};

	auto ptr = texture.get();
	EXPECT_NE(ptr, false);

	std::array<terraformer::rgba_pixel, 2> pixels{};
	ptr.upload(terraformer::span_2d<terraformer::rgba_pixel const>{1, 2, pixels.data()});

	auto const value = ptr.get_if<dummy_texture<0> const>();
	EXPECT_EQ(value->pixels.width(), 1);
	EXPECT_EQ(value->pixels.height(), 2);
	EXPECT_EQ(value->pixels.data(), pixels.data());
}