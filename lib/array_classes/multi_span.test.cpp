//@	{"target":{"name":"multi_span.test"}}

#include "./multi_span.hpp"

#include <testfwk/testfwk.hpp>
#include <array>

TESTCASE(terraformer_multi_span_access_element_by_index)
{
	std::array array_0{1, 2, 3, 4};
	std::array array_1{1.5, 2.5, 3.5};
	std::array<std::string, 3> array_2{"Foo", "Bar", "Kaka"};

	terraformer::multi_span span{
		terraformer::tuple{
			std::data(array_0),
			std::data(array_1),
			std::data(array_2)
		},
		std::size(array_0)
	};

	auto item = span[span.first_element_index() + 1];
	EXPECT_EQ(&item.template get<0>(), &array_0[1]);
	EXPECT_EQ(&item.template get<1>(), &array_1[1]);
	EXPECT_EQ(&item.template get<2>(), &array_2[1]);
}