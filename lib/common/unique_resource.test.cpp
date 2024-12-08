//@	{"target":{"name":"unique_resource.test"}}

#include "./unique_resource.hpp"

#include <testfwk/testfwk.hpp>
#include <testfwk/lifetime_checker.hpp>

namespace
{
	struct dummy_vtable
	{
		template<class T>
		constexpr dummy_vtable(T){}
	};
}

TESTCASE(terraformer_unique_resource_default_state_is_falsy)
{
	terraformer::unique_resource<dummy_vtable> res;
	EXPECT_EQ(res, false);
}

TESTCASE(terraformer_unique_resource_default_state_points_to_nullptr)
{
	terraformer::unique_resource<dummy_vtable> res;
	EXPECT_EQ(res.get_pointer(), nullptr);
}

TESTCASE(terraformer_unique_resource_non_trivial_resource_is_properly_destroyed)
{
	testfwk::lifetime_checker<int>::expect_ctor(1);
	terraformer::unique_resource<dummy_vtable> res{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};
}

TESTCASE(terraformer_unique_resource_constructed_is_truthy)
{
	testfwk::lifetime_checker<int>::expect_ctor(1);
	terraformer::unique_resource<dummy_vtable> res{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};
	EXPECT_EQ(res, true);
}

TESTCASE(terraformer_unique_resource_constructed_resource_points_to_non_null)
{
	testfwk::lifetime_checker<int>::expect_ctor(1);
	terraformer::unique_resource<dummy_vtable> res{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};
	EXPECT_NE(res.get_pointer(), nullptr);
}

TESTCASE(terraformer_unique_resource_constructed_resource_points_to_correct_vtable_type)
{
	testfwk::lifetime_checker<int>::expect_ctor(1);
	terraformer::unique_resource<dummy_vtable> res{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};
	auto vt = res.get_vtable();
	EXPECT_EQ((std::is_same_v<decltype(vt), dummy_vtable>), true);
}

TESTCASE(terraformer_unique_resource_same_vt_type_points_to_same_vt)
{
	testfwk::lifetime_checker<int>::expect_ctor(2);
	terraformer::unique_resource<dummy_vtable> res_1{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};
	terraformer::unique_resource<dummy_vtable> res_2{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};

	EXPECT_EQ(&res_1.get_vtable(), &res_2.get_vtable());
}

TESTCASE(terraformer_unique_resource_move_construct_moves_pointers_to_new_owner)
{
	testfwk::lifetime_checker<int>::expect_ctor(1);
	terraformer::unique_resource<dummy_vtable> res_1{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};

	auto orig_ptr = res_1.get_pointer();
	auto orig_vptr = &res_1.get_vtable();

	auto res_2 = std::move(res_1);

	EXPECT_EQ(res_1.get_pointer(), nullptr);
	EXPECT_EQ(res_1, false);
	EXPECT_EQ(res_2.get_pointer(), orig_ptr);
	EXPECT_EQ(&res_2.get_vtable(), orig_vptr);
}

TESTCASE(terraformer_unique_resource_move_assign_moves_pointers_to_new_owner_and_destroys_old_object)
{
	testfwk::lifetime_checker<int>::expect_ctor(1);
	terraformer::unique_resource<dummy_vtable> res_1{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};

	testfwk::lifetime_checker<int>::expect_ctor(1);
	terraformer::unique_resource<dummy_vtable> res_2{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};

	auto orig_ptr = res_1.get_pointer();
	auto orig_vptr = &res_1.get_vtable();

	res_2 = std::move(res_1);

	EXPECT_EQ(res_1.get_pointer(), nullptr);
	EXPECT_EQ(res_1, false);
	EXPECT_EQ(res_2.get_pointer(), orig_ptr);
	EXPECT_EQ(&res_2.get_vtable(), orig_vptr);
}

TESTCASE(terraformer_unqiue_resource_reset_reverts_state_to_default)
{
	testfwk::lifetime_checker<int>::expect_ctor(1);
	terraformer::unique_resource<dummy_vtable> res{
		std::in_place_type_t<testfwk::lifetime_checker<int>>{}
	};

	EXPECT_NE(res.get_pointer(), nullptr);
	res.reset();
	EXPECT_EQ(res.get_pointer(), nullptr);
}