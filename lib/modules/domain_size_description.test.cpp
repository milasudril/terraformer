//@	{"target":{"name":"domain_size_description.test"}}

#include "./domain_size_description.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_domain_size_image_size)
{
	auto const val = image_size(terraformer::domain_size_description{
		.width = terraformer::domain_length{1024.0f},
		.height = terraformer::domain_length{512.0f},
		.number_of_pixels = terraformer::pixel_count{4*1024*512}
	});

	EXPECT_EQ(val.width, 2048);
	EXPECT_EQ(val.height, 1024);
}

TESTCASE(terraformer_domain_size_pixel_size)
{
	auto const val = pixel_size(terraformer::domain_size_description{
		.width = terraformer::domain_length{1024.0f},
		.height = terraformer::domain_length{512.0f},
		.number_of_pixels = terraformer::pixel_count{4*1024*512}
	});

	EXPECT_EQ(val, 0.5f);
}

TESTCASE(terraformer_domain_size_image_width)
{
	auto const val = image_width(terraformer::domain_size_description{
		.width = terraformer::domain_length{1024.0f},
		.height = terraformer::domain_length{512.0f},
		.number_of_pixels = terraformer::pixel_count{4*1024*512}
	});

	EXPECT_EQ(val, 2048);
}

TESTCASE(terraformer_domain_size_image_height)
{
	auto const val = image_height(terraformer::domain_size_description{
		.width = terraformer::domain_length{1024.0f},
		.height = terraformer::domain_length{512.0f},
		.number_of_pixels = terraformer::pixel_count{4*1024*512}
	});

	EXPECT_EQ(val, 1024);
}

namespace
{
	struct test_form
	{
		template<class FieldDescriptor>
		void insert(FieldDescriptor&&)
		{
			// TODO: add some tests here
		}
	};
}

TESTCASE(terraformer_domain_size_bind_to_form)
{
	terraformer::domain_size_description dom_size{
		.width = terraformer::domain_length{1024.0f},
		.height = terraformer::domain_length{512.0f},
		.number_of_pixels = terraformer::pixel_count{4*1024*512}
	};

	test_form form;
	bind(form, std::ref(dom_size));
}