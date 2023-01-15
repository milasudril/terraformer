//@	{"target":{"name":"span_2d.test"}}

#include "./span_2d.hpp"
#include "./utils.hpp"

#include "testfwk/testfwk.hpp"

static_assert(terraformer::map_2d<terraformer::span_2d<float const>, float>);