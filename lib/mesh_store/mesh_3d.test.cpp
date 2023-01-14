//@	{"target":{"name":"mesh_3d.test"}}

#include "./mesh_3d.hpp"

#include "testfwk/testfwk.hpp"

static_assert(std::is_same_v<std::tuple_element_t<0, terraformer::vertex>, terraformer::location>);
/*static_assert(std::is_same_v<std::tuple_element_t<1, terraformer::vertex>, terraformer::direction>);*/