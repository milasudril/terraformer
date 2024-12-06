//@	{"target":{"name":"global_instance_counter.test"}}

#include "./global_instance_counter.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_global_instance_counter_objects_get_unique_ids)
{
	terraformer::global_instance_counter obj;
	EXPECT_EQ(obj.get_global_id(), 0);
	auto other = obj;
	EXPECT_EQ(other.get_global_id(), 1);
	auto moved_to = std::move(obj);
	EXPECT_EQ(moved_to.get_global_id(), 2);

	terraformer::global_instance_counter more_objects;
	EXPECT_EQ(more_objects.get_global_id(), 3);
}