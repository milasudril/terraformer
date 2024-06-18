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