//@	{"target":{"name":"composite_function.test"}}

#include "./composite_function.hpp"

#include <testfwk/testfwk.hpp>

#include <cmath>

TESTCASE(terraformer_composit_function_call)
{
	terraformer::composite_function const f{
		[](int a) {
			return a*a;
		},
		[](int b){
			return std::to_string(b - 10);
		},
		[](std::string&& str){
			str.append(" Foobar");
			return std::move(str);
		}
	};

	auto res = f(10);
	EXPECT_EQ(res, "90 Foobar");
}