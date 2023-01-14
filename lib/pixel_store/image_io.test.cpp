//@	{
//@	 "target":{"name":"image_io.test"}
//@	}

#include "./image_io.hpp"

#include "testfwk/testfwk.hpp"

TESTCASE(terraformer_image_load_rgb_image)
{
	constexpr auto zero_threshold = 2.0f*std::numeric_limits<float>::epsilon();

	auto const image = load(std::type_identity<terraformer::image>{}, "testdata/img_rgb.exr");
	EXPECT_EQ(image(0, 0).red(), 1.0f);
	EXPECT_LT(image(0, 0).green(), zero_threshold);
	EXPECT_LT(image(0, 0).blue(),  zero_threshold);
	EXPECT_EQ(image(0, 0).alpha(), 1.0f);

	EXPECT_LT(image(1, 0).red(), zero_threshold);
	EXPECT_EQ(image(1, 0).green(), 1.0f);
	EXPECT_LT(image(1, 0).blue(), zero_threshold);
	EXPECT_EQ(image(1, 0).alpha(), 1.0f);

	EXPECT_LT(image(2, 0).red(), zero_threshold);
	EXPECT_LT(image(2, 0).green(), zero_threshold);
	EXPECT_EQ(image(2, 0).blue(), 1.0f);
	EXPECT_EQ(image(2, 0).alpha(), 1.0f);

	EXPECT_LT(image(0, 1).red(), zero_threshold);
	EXPECT_EQ(image(0, 1).green(), 1.0f);
	EXPECT_EQ(image(0, 1).blue(), 1.0f);
	EXPECT_EQ(image(0, 1).alpha(), 1.0f);

	EXPECT_EQ(image(1, 1).red(), 1.0f);
	EXPECT_LT(image(1, 1).green(), zero_threshold);
	EXPECT_EQ(image(1, 1).blue(), 1.0f);
	EXPECT_EQ(image(1, 1).alpha(), 1.0f);

	EXPECT_EQ(image(2, 1).red(), 1.0f);
	EXPECT_EQ(image(2, 1).green(), 1.0f);
	EXPECT_LT(image(2, 1).blue(), zero_threshold);
	EXPECT_EQ(image(2, 1).alpha(), 1.0f);
}

TESTCASE(terraformer_image_load_rgba_image)
{
	constexpr auto zero_threshold = 2.0f*std::numeric_limits<float>::epsilon();

	auto const image = load(std::type_identity<terraformer::image>{}, "testdata/img_rgba.exr");
	EXPECT_EQ(image(0, 0).red(), 0.128401771f);
	EXPECT_LT(image(0, 0).green(), zero_threshold);
	EXPECT_LT(image(0, 0).blue(), zero_threshold);
	EXPECT_EQ(image(0, 0).alpha(), 0.128401771f);

	EXPECT_LT(image(1, 0).red(), zero_threshold);
	EXPECT_EQ(image(1, 0).green(), 0.250178099f);
	EXPECT_LT(image(1, 0).blue(), zero_threshold);
	EXPECT_EQ(image(1, 0).alpha(), 0.250178099f);
}

TESTCASE(terraformer_image_store_load_rgba_image)
{
	terraformer::image img_a{3, 2};
	img_a(0, 0) = terraformer::rgba_pixel{1.0f, 0.0f, 0.0f, 0.125f};
	img_a(1, 0) = terraformer::rgba_pixel{0.0f, 1.0f, 0.0f, 0.025f};
	img_a(2, 0) = terraformer::rgba_pixel{0.0f, 0.0f, 1.0f, 0.375f};
	img_a(0, 1) = terraformer::rgba_pixel{0.0f, 1.0f, 1.0f, 0.5f};
	img_a(1, 1) = terraformer::rgba_pixel{1.0f, 0.0f, 1.0f, 0.625f};
	img_a(2, 1) = terraformer::rgba_pixel{1.0f, 1.0f, 0.0f, 0.75f};

	auto id_string = std::string{MAIKE_BUILDINFO_TARGETDIR "/"};
	id_string += std::to_string(MAIKE_TASKID);
	store(img_a, id_string.c_str());

	try
	{
		auto const img_b = load(std::type_identity<terraformer::grayscale_image>{},
			id_string.c_str());
		abort();
	}
	catch(std::exception const& e)
	{
		EXPECT_EQ(std::string_view{e.what()}, "Expected a grayscale image");
	}

	auto const img_b = load(std::type_identity<terraformer::image>{}, id_string.c_str());

	EXPECT_EQ((std::ranges::equal(img_a.pixels(), img_b.pixels(), [](auto a, auto b) {
		return a.red() == b.red() && a.green() == b.green() && a.blue() == b.blue()
			&& a.alpha() == b.alpha();
	})), true);
}

TESTCASE(terraformer_image_store_load_grayscale_image)
{
	terraformer::grayscale_image img_a{3, 2};
	img_a(0, 0) = 0.125f;
	img_a(1, 0) = 0.025f;
	img_a(2, 0) = 0.375f;
	img_a(0, 1) = 0.5f;
	img_a(1, 1) = 0.625f;
	img_a(2, 1) = 0.75f;

	auto id_string = std::string{MAIKE_BUILDINFO_TARGETDIR "/"};
	id_string += std::to_string(MAIKE_TASKID);
	store(img_a, id_string.c_str());

	try
	{
		auto const img_b = load(std::type_identity<terraformer::image>{}, id_string.c_str());
		abort();
	}
	catch(std::exception const& e)
	{
		EXPECT_EQ(std::string_view{e.what()}, "Expected a color image");
	}

	auto const img_b = load(std::type_identity<terraformer::grayscale_image>{}, id_string.c_str());

	EXPECT_EQ((std::ranges::equal(img_a.pixels(), img_b.pixels(), [](auto a, auto b) {
		return a == b;
	})), true);
}
