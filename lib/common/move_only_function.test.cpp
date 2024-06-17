//@	{"target":{"name":"move_only_function.test.cpp"}}

#include "./move_only_function.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_move_only_function_default_state)
{
	terraformer::move_only_function<void()> obj;
	EXPECT_EQ(obj, false);
}

TESTCASE(terraformer_move_only_function_no_operation)
{
	terraformer::move_only_function<void(int)> obj{terraformer::no_operation_tag{}};

	REQUIRE_EQ(obj, true);
	obj(123);
}

namespace
{
	int do_stuff(int x)
	{ return 2*x; }
}

TESTCASE(terraformer_move_only_function_stateless_callback)
{
	terraformer::move_only_function<int(int)> obj{do_stuff};

	REQUIRE_EQ(obj, true);
	auto const val = obj(3);
	EXPECT_EQ(val, 6);
}

TESTCASE(terraformer_move_only_function_stateful_callback)
{
	int callcount = 0;
	terraformer::move_only_function<void()> obj{
		[&callcount](){
			++callcount;
		}
	};

	REQUIRE_EQ(obj, true);
	for(int k = 0; k != 3; ++k)
	{
		EXPECT_EQ(callcount, k);
		obj();
	}
	EXPECT_EQ(callcount, 3);
}

namespace
{
	struct callable_with_annotated_dtor
	{
		~callable_with_annotated_dtor()
		{ ++dtor_count.get(); }

		std::reference_wrapper<int> callcount;
		std::reference_wrapper<int> dtor_count;

		void operator()()
		{ ++callcount.get(); }
	};
}

TESTCASE(terraformer_move_only_function_reset)
{
	// Because we create a temporary callable_with_annotated_dtor, which is destroyed after it has
	// been moved to the heap.
	int dtor_count = -1;

	int callcount = 0;
	terraformer::move_only_function<void()> obj{callable_with_annotated_dtor{callcount, dtor_count}};

	REQUIRE_EQ(obj, true);
	for(int k = 0; k != 3; ++k)
	{
		EXPECT_EQ(callcount, k);
		EXPECT_EQ(dtor_count, 0);
		obj();
	}
	EXPECT_EQ(callcount, 3);
	EXPECT_EQ(dtor_count, 0);

	obj.reset();
	EXPECT_EQ(obj, false);
	EXPECT_EQ(dtor_count, 1);
}

TESTCASE(terraformer_move_only_function_move_construct)
{
	// Because we create a temporary callable_with_annotated_dtor, which is destroyed after it has
	// been moved to the heap.
	int dtor_count = -1;
	int callcount = 0;
	terraformer::move_only_function<void()> obj{callable_with_annotated_dtor{callcount, dtor_count}};

	REQUIRE_EQ(obj, true);
	for(int k = 0; k != 3; ++k)
	{
		EXPECT_EQ(callcount, k);
		EXPECT_EQ(dtor_count, 0);
		obj();
	}
	EXPECT_EQ(callcount, 3);
	EXPECT_EQ(dtor_count, 0);

	auto other = std::move(obj);
	EXPECT_EQ(obj, false);
	EXPECT_EQ(dtor_count, 0);
	REQUIRE_EQ(other, true);
	for(int k = 0; k != 3; ++k)
	{
		EXPECT_EQ(callcount, k + 3);
		EXPECT_EQ(dtor_count, 0);
		other();
	}
	EXPECT_EQ(callcount, 6);
	EXPECT_EQ(dtor_count, 0);
}

TESTCASE(terraformer_move_only_function_move_assign)
{
	// Because we create a temporary callable_with_annotated_dtor, which is destroyed after it has
	// been moved to the heap.
	int dtor_count_a = -1;
	int callcount_a = 0;
	terraformer::move_only_function<void()> obj_a{callable_with_annotated_dtor{callcount_a, dtor_count_a}};

	int dtor_count_b = -1;
	int callcount_b = 0;
	terraformer::move_only_function<void()> obj_b{callable_with_annotated_dtor{callcount_b, dtor_count_b}};

	REQUIRE_EQ(obj_a, true);
	for(int k = 0; k != 3; ++k)
	{
		EXPECT_EQ(callcount_a, k);
		EXPECT_EQ(dtor_count_a, 0);
		obj_a();
	}
	EXPECT_EQ(callcount_a, 3);
	EXPECT_EQ(dtor_count_a, 0);

	REQUIRE_EQ(obj_b, true);
	for(int k = 0; k != 2; ++k)
	{
		EXPECT_EQ(callcount_b, k);
		EXPECT_EQ(dtor_count_b, 0);
		obj_b();
	}
	EXPECT_EQ(callcount_b, 2);
	EXPECT_EQ(dtor_count_b, 0);

	obj_a = std::move(obj_b);
	EXPECT_EQ(dtor_count_a, 1);
	EXPECT_EQ(dtor_count_b, 0);
	EXPECT_EQ(obj_b, false);

	REQUIRE_EQ(obj_a, true);
	for(int k = 0; k != 3; ++k)
	{
		EXPECT_EQ(callcount_a, 3);
		EXPECT_EQ(dtor_count_a, 1);
		EXPECT_EQ(callcount_b, k + 2);
		EXPECT_EQ(dtor_count_b, 0);
		obj_a();
	}
	EXPECT_EQ(callcount_a, 3);
	EXPECT_EQ(dtor_count_a, 1);
	EXPECT_EQ(callcount_b, 5);
	EXPECT_EQ(dtor_count_b, 0);
}