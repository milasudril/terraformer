//@	{"target":{"name":"value_accessor.test"}}

#include "./value_accessor.hpp"

#include <memory>

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_common_value_accessor_tempval)
{
	auto res = terraformer::value_of(std::make_unique<int>(5));
	static_assert(std::is_same_v<decltype(res), std::unique_ptr<int>>);
	EXPECT_EQ(*res, 5);
}

TESTCASE(terraformer_common_value_accessor_not_tempval)
{
	auto val = std::make_unique<int>(5);
	auto& res = terraformer::value_of(val);
	static_assert(std::is_same_v<decltype(res), std::unique_ptr<int>&>);
	EXPECT_EQ(val.get(), res.get());
	EXPECT_EQ(*res, 5);
}

TESTCASE(terraformer_common_value_accessor_not_tempval_const)
{
	auto const val = std::make_unique<int>(5);
	auto& res = terraformer::value_of(val);
	static_assert(std::is_same_v<decltype(res), std::unique_ptr<int> const&>);
	EXPECT_EQ(val.get(), res.get());
	EXPECT_EQ(*res, 5);
}

TESTCASE(terraformer_common_value_accessor_ref_wrapper)
{
	int x = 5;
	auto& val = terraformer::value_of(std::ref(x));
	EXPECT_EQ(&val, &x);
	EXPECT_EQ(val, 5);
}