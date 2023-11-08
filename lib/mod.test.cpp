//@	{"target":{"name":"mod.test"}}

#include "./mod.hpp"

#include <testfwk/testfwk.hpp>

#include <unordered_map>

TESTCASE(terraformer_mod_float)
{
	std::array<float, 16> vals{
		0.0f,
		0.25f,
		0.5f,
		0.75f,
		1.0f,
		1.25f,
		1.5f,
		1.75f,
		2.0f,
		2.25f,
		2.5f,
		2.75f,
		3.0f,
		3.25f,
		3.5f,
		3.75f,
	};

	for(size_t k = 0; k != 65; ++k)
	{
		auto const x = 0.25f*(static_cast<float>(k) - 32);
		EXPECT_EQ(terraformer::mod(x, 4.0f), vals[k%16]);
   }
}

TESTCASE(terraformer_mod_int)
{
	std::unordered_map<int32_t, int32_t> correct_values
	{
		std::pair{-31, 8},
		std::pair{-30, 9},
		std::pair{-29, 10},
		std::pair{-28, 11},
		std::pair{-27, 12},
		std::pair{-26, 0},
		std::pair{-25, 1},
		std::pair{-24, 2},
		std::pair{-23, 3},
		std::pair{-22, 4},
		std::pair{-21, 5},
		std::pair{-20, 6},
		std::pair{-19, 7},
		std::pair{-18, 8},
		std::pair{-17, 9},
		std::pair{-16, 10},
		std::pair{-15, 11},
		std::pair{-14, 12},
		std::pair{-13, 0},
		std::pair{-12, 1},
		std::pair{-11, 2},
		std::pair{-10, 3},
		std::pair{-9, 4},
		std::pair{-8, 5},
		std::pair{-7, 6},
		std::pair{-6, 7},
		std::pair{-5, 8},
		std::pair{-4, 9},
		std::pair{-3, 10},
		std::pair{-2, 11},
		std::pair{-1, 12},
		std::pair{0, 0},
		std::pair{1, 1},
		std::pair{2, 2},
		std::pair{3, 3},
		std::pair{4, 4},
		std::pair{5, 5},
		std::pair{6, 6},
		std::pair{7, 7},
		std::pair{8, 8},
		std::pair{9, 9},
		std::pair{10, 10},
		std::pair{11, 11},
		std::pair{12, 12},
		std::pair{13, 0},
		std::pair{14, 1},
		std::pair{15, 2},
		std::pair{16, 3},
		std::pair{17, 4},
		std::pair{18, 5},
		std::pair{19, 6},
		std::pair{20, 7},
		std::pair{21, 8},
		std::pair{22, 9},
		std::pair{23, 10}
	};


	for(int k = -31; k <= 23; ++k)
	{
		EXPECT_EQ(terraformer::mod(k, 13), correct_values.at(k))
	}
}