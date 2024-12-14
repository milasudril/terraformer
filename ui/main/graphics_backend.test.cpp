//@	{"target":{"name": "graphics_backend.test"}}

#include "./graphics_backend.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct dummy_texture
	{
		void upload(terraformer::span_2d<terraformer::rgba_pixel const>)
		{}
	};

	struct dummy_backend
	{
		terraformer::ui::main::generic_unique_texture create(
			std::type_identity<terraformer::ui::main::generic_unique_texture>,
			uint64_t backend_id,
			terraformer::image const& img
		)
		{
			called_with_id = backend_id;
			called_with_image = &img;
			return terraformer::ui::main::generic_unique_texture{std::type_identity<dummy_texture>{}};
		}

		uint64_t called_with_id{static_cast<uint64_t>(-1)};
		terraformer::image const* called_with_image{nullptr};
	};
}

TESTCASE(terraformer_ui_main_graphics_backend_create_texture_from_image)
{
	dummy_backend my_backend;
	terraformer::ui::main::graphics_backend backend{my_backend};

	terraformer::image img{1,1};

	auto texture = backend.create(
		std::type_identity<terraformer::ui::main::generic_unique_texture>{},
		img
	);

	REQUIRE_NE(backend.get_global_id(), static_cast<uint64_t>(-1));
	EXPECT_EQ(my_backend.called_with_id, backend.get_global_id());
	EXPECT_EQ(my_backend.called_with_image, &img);
}