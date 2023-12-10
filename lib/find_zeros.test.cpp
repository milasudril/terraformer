//@	{"target":{"name":"find_zeros.test"}}

#include "./find_zeros.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_find_zeros_cosine)
{
	std::array<float, 64> data_points;
	auto const dx = 1.0f/16.0f;
	auto const f = 1.0f;
	for(size_t k = 0; k != std::size(data_points); ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		data_points[k] = std::cos(2.0f*std::numbers::pi_v<float>*x*f);
	}

	auto const zeros = terraformer::find_zeros(data_points);
	EXPECT_EQ(std::size(zeros), 2*std::size(data_points)/16);
	for(size_t k = 0; k != std::size(zeros); ++k)
	{
		auto const index = zeros[k];
		REQUIRE_GT(index, 0);
		auto const val_a = data_points[index - 1];
		auto const val_b = data_points[index];
		if(val_a >= 0.0f)
		{ EXPECT_LT(val_b, 0); }
		else
		{ EXPECT_GE(val_b, 0.0f); }
	}
}

TESTCASE(terraformer_find_zeros_sine)
{
	std::array<float, 64> data_points;
	auto const dx = 1.0f/16.0f;
	auto const f = 1.0f;
	for(size_t k = 0; k != std::size(data_points); ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		data_points[k] = std::sin(2.0f*std::numbers::pi_v<float>*x*f);
	}

	auto const zeros = terraformer::find_zeros(data_points);
	EXPECT_EQ(std::size(zeros), 2*std::size(data_points)/16);
	for(size_t k = 0; k != std::size(zeros); ++k)
	{
		auto const index = zeros[k];
		if(index > 0)
		{
			auto const val_a = data_points[index - 1];
			auto const val_b = data_points[index];
			if(val_a >= 0.0f)
			{ EXPECT_LT(val_b, 0); }
			else
			{ EXPECT_GE(val_b, 0.0f); }
		}
		else
		{ EXPECT_EQ(data_points[index], 0.0f); }
	}
}

TESTCASE(terraformer_find_zeros_small_oscillations_on_step_wave_cosine_cosine)
{
	std::array<float, 256> data_points;
	auto const dx = 1.0f/128.0f;
	auto const f1 = 1.0f;
	auto const f2 = 16.0f;
	for(size_t k = 0; k != std::size(data_points); ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const wave_val = std::cos(2.0f*std::numbers::pi_v<float>*x*f1);
		data_points[k] = 0.25f*std::cos(2.0f*std::numbers::pi_v<float>*x*f2)
			+ static_cast<float>(static_cast<int>(wave_val >= 0.0f? wave_val+ 0.5f: wave_val - 0.5f));
	}

	auto const zeros = terraformer::find_zeros(data_points);
	EXPECT_EQ(std::size(zeros), 2*std::size(data_points)/128);
	for(size_t k = 0; k != std::size(zeros); ++k)
	{
		auto const index = zeros[k];
		REQUIRE_GT(index, 0);
		auto const val_a = data_points[index - 1];
		auto const val_b = data_points[index];
		if(val_a >= 0.0f)
		{ EXPECT_LT(val_b, 0); }
		else
		{ EXPECT_GE(val_b, 0.0f); }
	}
}

TESTCASE(terraformer_find_zeros_small_oscillations_on_step_wave_cosine_sine)
{
	std::array<float, 256> data_points;
	auto const dx = 1.0f/128.0f;
	auto const f1 = 1.0f;
	auto const f2 = 16.0f;
	for(size_t k = 0; k != std::size(data_points); ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const wave_val = std::cos(2.0f*std::numbers::pi_v<float>*x*f1);
		data_points[k] = 0.25f*std::sin(2.0f*std::numbers::pi_v<float>*x*f2)
			+ static_cast<float>(static_cast<int>(wave_val >= 0.0f? wave_val+ 0.5f: wave_val - 0.5f));
	}

	auto const zeros = terraformer::find_zeros(data_points);
	EXPECT_EQ(std::size(zeros), 2*std::size(data_points)/128);
	for(size_t k = 0; k != std::size(zeros); ++k)
	{
		auto const index = zeros[k];
		REQUIRE_GT(index, 0);
		auto const val_a = data_points[index - 1];
		auto const val_b = data_points[index];
		if(val_a >= 0.0f)
		{ EXPECT_LT(val_b, 0); }
		else
		{ EXPECT_GE(val_b, 0.0f); }
	}
}

TESTCASE(terraformer_find_zeros_small_oscillations_on_step_wave_sine_cosine)
{
	std::array<float, 256> data_points;
	auto const dx = 1.0f/128.0f;
	auto const f1 = 1.0f;
	auto const f2 = 16.0f;
	for(size_t k = 0; k != std::size(data_points); ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const wave_val = std::sin(2.0f*std::numbers::pi_v<float>*x*f1);
		data_points[k] = 0.25f*std::cos(2.0f*std::numbers::pi_v<float>*x*f2)
			+ static_cast<float>(static_cast<int>(wave_val >= 0.0f? wave_val+ 0.5f: wave_val - 0.5f));
	}

	auto const zeros = terraformer::find_zeros(data_points);
	EXPECT_EQ(std::size(zeros), 2*std::size(data_points)/128);
	for(size_t k = 0; k != std::size(zeros); ++k)
	{
		auto const index = zeros[k];
		REQUIRE_GT(index, 0);
		auto const val_a = data_points[index - 1];
		auto const val_b = data_points[index];
		if(val_a >= 0.0f)
		{ EXPECT_LT(val_b, 0); }
		else
		{ EXPECT_GE(val_b, 0.0f); }
	}
}

TESTCASE(terraformer_find_zeros_small_oscillations_on_step_wave_sine_sine)
{
	std::array<float, 512> data_points;
	auto const dx = 1.0f/256.0f;
	auto const f1 = 1.0f;
	auto const f2 = 16.0f;
	for(size_t k = 0; k != std::size(data_points); ++k)
	{
		auto const x = static_cast<float>(k)*dx;
		auto const wave_val = std::sin(2.0f*std::numbers::pi_v<float>*x*f1);
		data_points[k] = 0.25f*std::sin(2.0f*std::numbers::pi_v<float>*x*f2)
			+ static_cast<float>(static_cast<int>(wave_val >= 0.0f? wave_val+ 0.5f: wave_val - 0.5f));
	}

	auto const zeros = terraformer::find_zeros(data_points);
	EXPECT_EQ(std::size(zeros), 2*std::size(data_points)/256);
	for(size_t k = 0; k != std::size(zeros); ++k)
	{
		auto const index = zeros[k];
		REQUIRE_GT(index, 0);
		auto const val_a = data_points[index - 1];
		auto const val_b = data_points[index];
		if(val_a >= 0.0f)
		{ EXPECT_LT(val_b, 0); }
		else
		{ EXPECT_GE(val_b, 0.0f); }
	}
}