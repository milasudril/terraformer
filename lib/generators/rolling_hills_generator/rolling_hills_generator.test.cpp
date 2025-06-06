//@	{"target":{"name":"rolling_hills_generator.test"}}

#include "./rolling_hills_generator.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_rolling_hills_generator_make_normalized_filter_descriptor_one_period_on_square_domain)
{
	{
		terraformer::domain_size_descriptor const dom_size{
			.width = 4096.0f,
			.height = 4096.0f
		};

		terraformer::rolling_hills_filter_descriptor const filter_in{
			.wavelength_x = 4096.0f,
			.wavelength_y = 4096.0f,
			.lf_rolloff = 2.0f,
			.hf_rolloff = 2.0f,
			.y_direction = 0.0f
		};

		auto const res = make_rolling_hills_normalized_filter_descriptor(dom_size, filter_in);

		EXPECT_EQ(res.width, 256);
		EXPECT_EQ(res.height, 256);

		auto const fs_x = static_cast<float>(res.width)/dom_size.width;
		auto const fs_y = static_cast<float>(res.height)/dom_size.height;
		EXPECT_EQ(fs_x, fs_y);
		EXPECT_EQ(fs_x, 0.0625f);
		EXPECT_EQ(fs_y, 0.0625f);

		auto const f_x = 1.0f/filter_in.wavelength_x;
		auto const f_y = 1.0f/filter_in.wavelength_y;
		auto const f_x_discrete = f_x*static_cast<float>(res.width)/fs_x;
		auto const f_y_discrete = f_y*static_cast<float>(res.height)/fs_y;

		EXPECT_EQ(res.f_x, f_x_discrete);
		EXPECT_EQ(res.f_y, f_y_discrete);
	}

	{
		terraformer::domain_size_descriptor const dom_size{
			.width = 8192.0f,
			.height = 8192.0f
		};

		terraformer::rolling_hills_filter_descriptor const filter_in{
			.wavelength_x = 8192.0f,
			.wavelength_y = 8192.0f,
			.lf_rolloff = 2.0f,
			.hf_rolloff = 2.0f,
			.y_direction = 0.0f
		};

		auto const res = make_rolling_hills_normalized_filter_descriptor(dom_size, filter_in);

		EXPECT_EQ(res.width, 256);
		EXPECT_EQ(res.height, 256);

		auto const fs_x = static_cast<float>(res.width)/dom_size.width;
		auto const fs_y = static_cast<float>(res.height)/dom_size.height;
		EXPECT_EQ(fs_x, fs_y);
		EXPECT_EQ(fs_x, 0.03125);
		EXPECT_EQ(fs_y, 0.03125);

		auto const f_x = 1.0f/filter_in.wavelength_x;
		auto const f_y = 1.0f/filter_in.wavelength_y;
		auto const f_x_discrete = f_x*static_cast<float>(res.width)/fs_x;
		auto const f_y_discrete = f_y*static_cast<float>(res.height)/fs_y;

		EXPECT_EQ(res.f_x, f_x_discrete);
		EXPECT_EQ(res.f_y, f_y_discrete);
	}
}

TESTCASE(terraformer_rolling_hills_generator_make_normalized_filter_descriptor_one_period_x_two_periods_y_on_square_domain)
{
	terraformer::domain_size_descriptor const dom_size{
		.width = 4096.0f,
		.height = 4096.0f
	};

	terraformer::rolling_hills_filter_descriptor const filter_in{
		.wavelength_x = 4096.0f,
		.wavelength_y = 2048.0f,
		.lf_rolloff = 2.0f,
		.hf_rolloff = 2.0f,
		.y_direction = 0.0f
	};

	auto const res = make_rolling_hills_normalized_filter_descriptor(dom_size, filter_in);

	EXPECT_EQ(res.width, 512);
	EXPECT_EQ(res.height, 512);

	auto const fs_x = static_cast<float>(res.width)/dom_size.width;
	auto const fs_y = static_cast<float>(res.height)/dom_size.height;
	EXPECT_EQ(fs_x, fs_y);
	EXPECT_EQ(fs_x, 0.125f);
	EXPECT_EQ(fs_y, 0.125f);

	auto const f_x = 1.0f/filter_in.wavelength_x;
	auto const f_y = 1.0f/filter_in.wavelength_y;
	auto const f_x_discrete = f_x*static_cast<float>(res.width)/fs_x;
	auto const f_y_discrete = f_y*static_cast<float>(res.height)/fs_y;

	EXPECT_EQ(res.f_y, 2.0f);
	EXPECT_EQ(res.f_x, f_x_discrete);
	EXPECT_EQ(res.f_y, f_y_discrete);
}

TESTCASE(terraformer_rolling_hills_generator_make_normalized_filter_descriptor_two_periods_x_one_period_y_on_square_domain)
{
	terraformer::domain_size_descriptor const dom_size{
		.width = 4096.0f,
		.height = 4096.0f
	};

	terraformer::rolling_hills_filter_descriptor const filter_in{
		.wavelength_x = 2048.0f,
		.wavelength_y = 4096.0f,
		.lf_rolloff = 2.0f,
		.hf_rolloff = 2.0f,
		.y_direction = 0.0f
	};

	auto const res = make_rolling_hills_normalized_filter_descriptor(dom_size, filter_in);

	EXPECT_EQ(res.width, 512);
	EXPECT_EQ(res.height, 512);

	auto const fs_x = static_cast<float>(res.width)/dom_size.width;
	auto const fs_y = static_cast<float>(res.height)/dom_size.height;
	EXPECT_EQ(fs_x, fs_y);
	EXPECT_EQ(fs_x, 0.125f);
	EXPECT_EQ(fs_y, 0.125f);

	auto const f_x = 1.0f/filter_in.wavelength_x;
	auto const f_y = 1.0f/filter_in.wavelength_y;
	auto const f_x_discrete = f_x*static_cast<float>(res.width)/fs_x;
	auto const f_y_discrete = f_y*static_cast<float>(res.height)/fs_y;

	EXPECT_EQ(res.f_x, 2.0f);
	EXPECT_EQ(res.f_x, f_x_discrete);
	EXPECT_EQ(res.f_y, f_y_discrete);
}

TESTCASE(terraformer_rolling_hills_generator_make_normalized_filter_descriptor_two_periods_x_one_period_y_on_rect_domain)
{
	terraformer::domain_size_descriptor const dom_size{
		.width = 8192.0f,
		.height = 4096.0f
	};

	terraformer::rolling_hills_filter_descriptor const filter_in{
		.wavelength_x = 4096.0f,
		.wavelength_y = 4096.0f,
		.lf_rolloff = 2.0f,
		.hf_rolloff = 2.0f,
		.y_direction = 0.0f
	};

	auto const res = make_rolling_hills_normalized_filter_descriptor(dom_size, filter_in);

	EXPECT_EQ(res.width, 512);
	EXPECT_EQ(res.height, 256);

	auto const fs_x = static_cast<float>(res.width)/dom_size.width;
	auto const fs_y = static_cast<float>(res.height)/dom_size.height;
	EXPECT_EQ(fs_x, fs_y);
	EXPECT_EQ(fs_x, 0.0625f);
	EXPECT_EQ(fs_y, 0.0625f);

	auto const f_x = 1.0f/filter_in.wavelength_x;
	auto const f_y = 1.0f/filter_in.wavelength_y;
	auto const f_x_discrete = f_x*static_cast<float>(res.width)/fs_x;
	auto const f_y_discrete = f_y*static_cast<float>(res.height)/fs_y;

	EXPECT_EQ(res.f_x, 2.0f);
	EXPECT_EQ(res.f_x, f_x_discrete);
	EXPECT_EQ(res.f_y, f_y_discrete);
}