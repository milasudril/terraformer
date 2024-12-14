//@	{"target":{"name":"graphics_backend_ref.test"}}

#include "./graphics_backend_ref.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct dummy_texture
	{
		explicit dummy_texture(terraformer::span_2d<terraformer::rgba_pixel const>){}

		void upload(terraformer::span_2d<terraformer::rgba_pixel const>) {}
		void bind(int){};
	};

	struct dummy_backend
	{
		constexpr uint64_t get_global_id() const
		{ return 124; }

		auto create(
			std::type_identity<terraformer::ui::main::texture>,
			uint64_t backend_id,
			terraformer::image const& img
		)
		{
			return terraformer::ui::main::texture{std::in_place_type_t<dummy_texture>{}, backend_id, img};
		}
	};
}

TESTCASE(terraformer_resource_backend_ref_create_texture)
{
	dummy_backend my_backend;
	terraformer::ui::main::graphics_backend_ref resource_factroy{my_backend};
	terraformer::image img{3, 2};
	auto texture = resource_factroy.create(
		std::type_identity<terraformer::ui::main::texture>{},
		img
	);

	EXPECT_EQ(texture.belongs_to_backend(my_backend.get_global_id()), true);
	EXPECT_EQ(texture.belongs_to_backend(3), false);
}

