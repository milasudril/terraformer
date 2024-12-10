//@	{"target":{"name":"graphics_resource_factory.test"}}

#include "./graphics_resource_factory.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct dummy_texture
	{
		explicit dummy_texture(terraformer::span_2d<terraformer::rgba_pixel const>){}

		void upload(terraformer::span_2d<terraformer::rgba_pixel const>) {}
		void bind(int){};
	};

	struct dummy_factory
	{
		constexpr uint64_t get_global_id() const
		{ return 124; }

		auto create(
			std::type_identity<terraformer::ui::main::texture>,
			uint64_t factory_id,
			terraformer::image const& img
		)
		{
			return terraformer::ui::main::texture{std::in_place_type_t<dummy_texture>{}, factory_id, img};
		}
	};
}

TESTCASE(terraformer_resource_factory_create_texture)
{
	dummy_factory my_factory;
	terraformer::ui::main::graphics_resource_factory resource_factroy{my_factory};
	terraformer::image img{3, 2};
	auto texture = resource_factroy.create(
		std::type_identity<terraformer::ui::main::texture>{},
		img
	);

	EXPECT_EQ(texture.created_by_factory(my_factory.get_global_id()), true);
	EXPECT_EQ(texture.created_by_factory(3), false);
}

