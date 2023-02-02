//@	{"target":{"name":"callable_tuple.test"}}

#include "./callable_tuple.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(callable_tuple_call)
{
	int const n = 4;
	terraformer::callable_tuple func{[](int x, int y, int z){
		EXPECT_EQ(x, 1);
		EXPECT_EQ(y, 2);
		EXPECT_EQ(z, 4);
		return 3;
	}, 1, 2, n};

	auto ret = func();
	EXPECT_EQ(ret, 3);
}