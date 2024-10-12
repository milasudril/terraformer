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
		for(auto k = span.first_element_index(); k != std::size(span); ++k)
		{
			EXPECT_EQ(span[k], data[l]);
			++l;
		}
	}

	{
		auto l = span.first_element_index();
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
}

TESTCASE(terraformer_span_find_next_wrap_around_empty_span)
{
	terraformer::span<int> span{};

	EXPECT_EQ(
		find_next_wrap_around(
			span,
			span.first_element_index() + 2,
			terraformer::span_search_direction::forward,
			[](auto){ return true; }
		),
		decltype(span)::npos
	);

	EXPECT_EQ(
		find_next_wrap_around(
			span,
			span.first_element_index() + 2,
			terraformer::span_search_direction::backward,
			[](auto){ return true; }
		),
		decltype(span)::npos
	);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forward_no_match)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.first_element_index() + 2;

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		[](auto){ return false; }
	);

	EXPECT_EQ(offset, decltype(span)::npos);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backward_no_match)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.first_element_index() + 2;

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		[](auto){ return false; }
	);

	EXPECT_EQ(offset, decltype(span)::npos);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forward_match_even)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.first_element_index() + 2;
	auto is_even = [](auto x){ return x % 2 == 0; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		is_even
	);
	EXPECT_EQ(offset, span.first_element_index() + 3);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		is_even
	);
	EXPECT_EQ(offset, span.first_element_index() + 1);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		is_even
	);
	EXPECT_EQ(offset, span.first_element_index() + 3);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backward_match_even)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.first_element_index() + 2;
	auto is_even = [](auto x){ return x % 2 == 0; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		is_even
	);
	EXPECT_EQ(offset, span.first_element_index() + 1);

	offset = find_next_wrap_around(
		span,
		offset - 1,
		terraformer::span_search_direction::backward,
		is_even
	);
	EXPECT_EQ(offset, span.first_element_index() + 3);

	offset = find_next_wrap_around(
		span,
		offset - 1,
		terraformer::span_search_direction::backward,
		is_even
	);
	EXPECT_EQ(offset, span.first_element_index() + 1);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forward_match_odd)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.first_element_index() + 2;
	auto is_odd = [](auto x){ return x % 2 != 0; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		is_odd
	);
	EXPECT_EQ(offset, span.first_element_index() + 4);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		is_odd
	);
	EXPECT_EQ(offset, span.first_element_index() + 0);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		is_odd
	);
	EXPECT_EQ(offset, span.first_element_index() + 2);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backward_match_odd)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.first_element_index() + 2;
	auto is_odd = [](auto x){ return x % 2 != 0; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		is_odd
	);
	EXPECT_EQ(offset, span.first_element_index() + 0);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		is_odd
	);
	EXPECT_EQ(offset, span.first_element_index() + 4);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		is_odd
	);
	EXPECT_EQ(offset, span.first_element_index() + 2);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forward_match_all)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.first_element_index() + 2;
	auto always_true = [](auto){ return true; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 3);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 4);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 0);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 1);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::forward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 2);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backward_match_all)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto offset = span.first_element_index() + 2;
	auto always_true = [](auto){ return true; };

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 1);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 0);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 4);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 3);

	offset = find_next_wrap_around(
		span,
		offset,
		terraformer::span_search_direction::backward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index() + 2);
}

TESTCASE(terraformer_span_find_next_wrap_around_step_backward_match_all_null_init)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto always_true = [](auto){ return true; };

	auto offset = find_next_wrap_around(
		span,
		decltype(span)::npos,
		terraformer::span_search_direction::backward,
		always_true
	);
	EXPECT_EQ(offset, span.last_element_index());
}

TESTCASE(terraformer_span_find_next_wrap_around_step_forward_match_all_null_init)
{
	std::array<int, 5> data{1, 2, 3, 4, 5};
	terraformer::span span{std::begin(data), std::end(data)};

	auto always_true = [](auto){ return true; };

	auto offset = find_next_wrap_around(
		span,
		decltype(span)::npos,
		terraformer::span_search_direction::forward,
		always_true
	);
	EXPECT_EQ(offset, span.first_element_index());
}