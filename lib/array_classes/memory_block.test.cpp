//@	{"target":{"name":"memory_block.test"}}

#include "./memory_block.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_memory_block_default_construct)
{
	terraformer::memory_block obj;
	EXPECT_EQ(static_cast<bool>(obj), false);
	EXPECT_EQ(obj.get(), nullptr);
	EXPECT_EQ(obj.interpret_as<int>(), nullptr);
}

TESTCASE(terraformer_memory_block_construct_with_size)
{
	terraformer::memory_block obj{make_byte_size(terraformer::array_size<int>{12})};
	EXPECT_EQ(static_cast<bool>(obj), true);
	EXPECT_NE(obj.get(), nullptr);
	EXPECT_NE(obj.interpret_as<int>(), nullptr);
}
