//@	{"target":{"name":"kdtree.test"}}

#include "./kdtree.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_kdtree_2d_build_from_one_point)
{
	std::vector const points{
		terraformer::location{.0f, .0f, 0.0f}
	};
	terraformer::kdtree_2d tree{std::vector{points}};
	size_t callcount = 0;
	tree.visit_nodes([&callcount](auto loc){
		EXPECT_EQ(loc[0], 0.0f);
		EXPECT_EQ(loc[1], 0.0f);
		++callcount;
	});
	EXPECT_EQ(callcount, std::size(points));
}

TESTCASE(terraformer_kdtree_2d_build_from_same_points)
{
	std::vector const points{
		terraformer::location{.0f, .0f, 0.0f},
		terraformer::location{.0f, .0f, 0.0f},
		terraformer::location{.0f, .0f, 0.0f},
		terraformer::location{.0f, .0f, 0.0f},
		terraformer::location{.0f, .0f, 0.0f},
		terraformer::location{.0f, .0f, 0.0f},
		terraformer::location{.0f, .0f, 0.0f},
	};

	terraformer::kdtree_2d tree{std::vector{points}};
	size_t callcount = 0;
	tree.visit_nodes([&callcount](auto loc){
		EXPECT_EQ(loc[0], 0.0f);
		EXPECT_EQ(loc[1], 0.0f);
		++callcount;
	});
	EXPECT_EQ(callcount, std::size(points));
}

TESTCASE(terraformer_kdtree_2d_build_and_search)
{
	std::vector const points{
		terraformer::location{7.0f, 2.0f, 0.0f},
		terraformer::location{9.0f, 6.0f, 0.0f},
		terraformer::location{5.0f, 4.0f, 0.0f},
		terraformer::location{4.0f, 7.0f, 0.0f},
		terraformer::location{8.0f, 1.0f, 0.0f},
		terraformer::location{2.0f, 3.0f, 0.0f},
	};

	std::vector const expected_points{
		terraformer::location{7.0f, 2.0f, 0.0f},
		terraformer::location{5.0f, 4.0f, 0.0f},
		terraformer::location{2.0f, 3.0f, 0.0f},
		terraformer::location{4.0f, 7.0f, 0.0f},
		terraformer::location{9.0f, 6.0f, 0.0f},
		terraformer::location{8.0f, 1.0f, 0.0f}
	};

	terraformer::kdtree_2d tree{std::vector{points}};
	size_t callcount = 0;
	tree.visit_nodes([&callcount, &expected_points](auto loc){
		EXPECT_EQ(loc, expected_points[callcount])
		++callcount;
	});
	EXPECT_EQ(callcount, std::size(points));

	{
		auto const res = tree.closest_point(terraformer::location{6.0f, 7.0f, 0.0f});
		EXPECT_EQ(res.loc, (terraformer::location{4.0f, 7.0f, 0.0f}));
	}

	{
		auto const res = tree.closest_point(terraformer::location{6.9f, 7.0f, 0.0f});
		EXPECT_EQ(res.loc, (terraformer::location{9.0f, 6.0f, 0.0f}));
	}
}