//@	{"target":{"name":"pixel.test"}}

#include "./rgba_pixel.hpp"

#include "testfwk/testfwk.hpp"

#include <type_traits>

static_assert(std::is_trivial_v<RgbaPixel>);
