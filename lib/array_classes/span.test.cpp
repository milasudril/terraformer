//@	{"target":{"name":"span.test"}}

#include "./span.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_span_test_all_members)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};

	terraformer::span span{std::begin(data), std::end(data)};

	EXPECT_EQ(std::size(span).get(), std::size(data));

	{
		size_t l = 0;
		for(auto k : span.element_indices())
		{
			EXPECT_EQ(span[k], data[l]);
			++l;
		}
	}

	{
		auto l = span.element_indices().front();
		EXPECT_EQ(l.get(), 0);
		auto ptr_a = std::data(span);
		auto ptr_b = std::data(data);
		while(l != std::size(span))
		{
			EXPECT_EQ(ptr_a, ptr_b);
			++l;
			++ptr_a;
			++ptr_b;
		}
	}

	{
		auto begin = std::begin(span);
		auto end = std::end(span);
		auto other = std::begin(data);

		while(begin != end)
		{
			EXPECT_EQ(begin, other);
			++begin;
			++other;
		}
	}

	auto clamp_left =  span(-1, terraformer::clamp_index{});
	EXPECT_EQ(clamp_left, 1);

	auto clamp_right = span(6, terraformer::clamp_index{});
	EXPECT_EQ(clamp_right, 5);
}

TESTCASE(terraformer_span_find_next_wrap_around_empty_span)
{
	terraformer::span<int> span{};

	EXPECT_EQ(
		find_next_wrap_around(
			span,
			span.element_indices().front() + 2,
			terraformer::span_search_direction::forwards,
			[](auto){ return true; }
		),
		decltype(span)::npos
	);

	EXPECT_EQ(
		find_next_wrap_around(
			span,
			span.element_indices().front() + 2,
			terraformer::span_search_direction::backwards,
			[](auto){ return true; }
		),
		decltype(span)::npos
	);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forwards_no_match)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.element_indices().front() + 2;

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		[](auto){ return false; }
	);

	EXPECT_EQ(offset, decltype(span)::npos);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backwards_no_match)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.element_indices().front() + 2;

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		[](auto){ return false; }
	);

	EXPECT_EQ(offset, decltype(span)::npos);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forwards_match_even)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.element_indices().front() + 2;
	auto is_even = [](auto x){ return x % 2 == 0; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		is_even
	);
	EXPECT_EQ(offset, span.element_indices().front() + 3);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		is_even
	);
	EXPECT_EQ(offset, span.element_indices().front() + 1);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		is_even
	);
	EXPECT_EQ(offset, span.element_indices().front() + 3);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backwards_match_even)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.element_indices().front() + 2;
	auto is_even = [](auto x){ return x % 2 == 0; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		is_even
	);
	EXPECT_EQ(offset, span.element_indices().front() + 1);

	offset = find_next_wrap_around(
		span,
		offset - 1,
		terraformer::span_search_direction::backwards,
		is_even
	);
	EXPECT_EQ(offset, span.element_indices().front() + 3);

	offset = find_next_wrap_around(
		span,
		offset - 1,
		terraformer::span_search_direction::backwards,
		is_even
	);
	EXPECT_EQ(offset, span.element_indices().front() + 1);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forwards_match_odd)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.element_indices().front() + 2;
	auto is_odd = [](auto x){ return x % 2 != 0; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		is_odd
	);
	EXPECT_EQ(offset, span.element_indices().front() + 4);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		is_odd
	);
	EXPECT_EQ(offset, span.element_indices().front() + 0);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		is_odd
	);
	EXPECT_EQ(offset, span.element_indices().front() + 2);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backwards_match_odd)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.element_indices().front() + 2;
	auto is_odd = [](auto x){ return x % 2 != 0; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		is_odd
	);
	EXPECT_EQ(offset, span.element_indices().front() + 0);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		is_odd
	);
	EXPECT_EQ(offset, span.element_indices().front() + 4);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		is_odd
	);
	EXPECT_EQ(offset, span.element_indices().front() + 2);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forwards_match_all)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.element_indices().front() + 2;
	auto always_true = [](auto){ return true; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 3);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 4);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 0);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 1);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 2);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backwards_match_all)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.element_indices().front() + 2;
	auto always_true = [](auto){ return true; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 1);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 0);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 4);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 3);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front() + 2);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backwards_match_all_null_init)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto always_true = [](auto){ return true; };

	auto offset = find_next_wrap_around(
		span,
		decltype(span)::npos,
		terraformer::span_search_direction::backwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().back());
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forwards_match_all_null_init)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto always_true = [](auto){ return true; };

	auto offset = find_next_wrap_around(
		span,
		decltype(span)::npos,
		terraformer::span_search_direction::forwards,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front());
}

TESTCASE(terraformer_span_find_next_wrap_around_no_step_null_init)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto always_true = [](auto){ return true; };

	auto offset = find_next_wrap_around(
		span,
		decltype(span)::npos,
		terraformer::span_search_direction::stay,
		always_true
	);
	EXPECT_EQ(offset, decltype(span)::npos);
}

TESTCASE(terraformer_span_find_next_wrap_around_no_step_nonnull_init)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto always_true = [](auto){ return true; };

	auto offset = find_next_wrap_around(
		span,
		span.element_indices().front(),
		terraformer::span_search_direction::stay,
		always_true
	);
	EXPECT_EQ(offset, span.element_indices().front());
}