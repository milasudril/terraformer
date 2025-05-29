//@	{"target":{"name":"plain.test"}}

#include "./plain.hpp"

#include "lib/pixel_store/image.hpp"
#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_plain_replace_pixels)
{
	terraformer::grayscale_image img{256, 192};

	replace_pixels(
		img.pixels(),
		4.0f,
		terraformer::plain_descriptor{
			.n = 0.0f,
			.ne = terraformer::plain_corner_descriptor{
				.elevation = 0.0f,
				.ddx = 1.0f,
				.ddy = 1.0f
			},
			.e = 0.0f,
			.se = terraformer::plain_corner_descriptor{
				.elevation = 0.0f,
				.ddx = 1.0f,
				.ddy = -1.0f
			},
			.s = 0.0f,
			.sw = terraformer::plain_corner_descriptor{
				.elevation = 0.0f,
				.ddx = -1.0f,
				.ddy = 1.0f
			},
			.w = 0.0f,
			.nw = terraformer::plain_corner_descriptor{
				.elevation = 0.0f,
				.ddx = -1.0f,
				.ddy = -1.0f
			},
		}
	);

	// TODO: Verdict store(img, "/dev/shm/slask.exr");
}