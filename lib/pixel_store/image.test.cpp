//@	{"target":{"name":"image.test"}}

#include "./image.hpp"
#include "./image_io.hpp"

#include "lib/common/span_2d.hpp"
#include "testfwk/testfwk.hpp"

#include <format>

TESTCASE(terraformer_image_create_from_data_block)
{
	terraformer::image img{3u, 2u};
	EXPECT_EQ(img.width(), 3);
	EXPECT_EQ(img.height(), 2);

	auto ptr   = img.pixels().begin();
	*(ptr + 0) = terraformer::red();
	*(ptr + 1) = terraformer::green();
	*(ptr + 2) = terraformer::blue();
	*(ptr + 3) = terraformer::cyan();
	*(ptr + 4) = terraformer::magenta();
	*(ptr + 5) = terraformer::yellow();

	EXPECT_EQ(img(0, 0).red(), 1.0f);
	EXPECT_EQ(img(0, 0).green(), 0.0f);
	EXPECT_EQ(img(0, 0).blue(), 0.0f);

	EXPECT_EQ(img(1, 0).red(), 0.0f);
	EXPECT_EQ(img(1, 0).green(), 1.0f);
	EXPECT_EQ(img(1, 0).blue(), 0.0f);

	EXPECT_EQ(img(2, 0).red(), 0.0f);
	EXPECT_EQ(img(2, 0).green(), 0.0f);
	EXPECT_EQ(img(2, 0).blue(), 1.0f);

	EXPECT_EQ(img(0, 1).red(), 0.0f);
	EXPECT_EQ(img(0, 1).green(), 1.0f);
	EXPECT_EQ(img(0, 1).blue(), 1.0f);

	EXPECT_EQ(img(1, 1).red(), 1.0f);
	EXPECT_EQ(img(1, 1).green(), 0.0f);
	EXPECT_EQ(img(1, 1).blue(), 1.0f);

	EXPECT_EQ(img(2, 1).red(), 1.0f);
	EXPECT_EQ(img(2, 1).green(), 1.0f);
	EXPECT_EQ(img(2, 1).blue(), 0.0f);
}

TESTCASE(terraformer_image_create_empty)
{
	terraformer::image img{3u, 2u};
	EXPECT_EQ(img.width(), 3);
	EXPECT_EQ(img.height(), 2);
}

TESTCASE(terraformer_image_floodfill)
{
	terraformer::grayscale_image output{1024, 1024};
	auto const t_start = std::chrono::steady_clock::now();
	floodfill(
		output.pixels(),
		terraformer::pixel_coordinates{
			.x = 256,
			.y = 512
		},
		[](auto...){
			return 1.0f;
		},
		[](uint32_t x, uint32_t y) {
			constexpr auto x_0 = 512.0f;
			constexpr auto y_0 = 512.0f;

			auto const x_float = static_cast<float>(x) - x_0;
			auto const y_float = static_cast<float>(y) - y_0;

			auto const r = std::sqrt(x_float*x_float + y_float*y_float);

			return r >= 192.0f && r<= 384.0f;
		}
	);
	auto const t_end = std::chrono::steady_clock::now();
	printf("Duration = %.8g\n", std::chrono::duration<double>(t_end - t_start).count());

	store(
		output,
		std::format("{}/{}_floodfill_test.exr", MAIKE_BUILDINFO_TARGETDIR, MAIKE_TASKID).c_str()
	);
}