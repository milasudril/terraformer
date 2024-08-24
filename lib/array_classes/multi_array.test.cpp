//@	{"target":{"name":"multi_array.test"}}

#include "./multi_array.hpp"

#include <cassert>
#include <testfwk/testfwk.hpp>
#include <testfwk/lifetime_checker.hpp>

namespace
{
	template<class T>
	using default_constructible_type = testfwk::lifetime_checker<T>;

	template<class T>
	class holder
	{
	public:
		explicit holder(T value):m_value{value}
		{}

		auto value() const
		{ return m_value; }

	private:
		T m_value;
	};

	template<class T>
	using no_default_constructible_type = testfwk::lifetime_checker<holder<T>>;
}

TESTCASE(terraformer_multi_array_generate_mem_blocks)
{
	auto blocks = generate_mem_blocks(terraformer::array_size<terraformer::multi_array_tag<int, double>>{12});
	EXPECT_EQ(std::size(blocks), 2);
	EXPECT_NE(blocks[0].get(), nullptr);
	EXPECT_NE(blocks[1].get(), nullptr);
}

TESTCASE(terraformer_mulite_array_uninitialized_copy)
{
	using size_type = terraformer::array_size<
		terraformer::multi_array_tag<
			default_constructible_type<int>,
			default_constructible_type<double>
		>
	>;

	auto blocks = generate_mem_blocks(size_type{3});

	EXPECT_EQ(std::size(blocks), 2);
	default_constructible_type<int>::expect_ctor(3);
	std::uninitialized_value_construct_n(blocks[0].interpret_as<default_constructible_type<int>>(), 3);

	default_constructible_type<double>::expect_ctor(3);
	std::uninitialized_value_construct_n(blocks[1].interpret_as<default_constructible_type<double>>(), 3);

	auto new_blocks = generate_mem_blocks(size_type{3});
	default_constructible_type<int>::expect_copy_ctor(3);
	default_constructible_type<double>::expect_copy_ctor(3);
	uninitialized_copy(blocks, new_blocks, size_type{3});

	std::destroy_n(blocks[0].interpret_as<default_constructible_type<int>>(), 3);
	std::destroy_n(blocks[1].interpret_as<default_constructible_type<double>>(), 3);

	std::destroy_n(new_blocks[0].interpret_as<default_constructible_type<int>>(), 3);
	std::destroy_n(new_blocks[1].interpret_as<default_constructible_type<double>>(), 3);
}

TESTCASE(terraformer_multi_array_reseve_on_empty)
{
	terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>
	> array;
	using size_type = decltype(array)::size_type;

	{
		REQUIRE_EQ(array.size(), size_type{});
		REQUIRE_EQ(array.capacity(), size_type{});
		auto const first_array = array.get<0>();
		REQUIRE_EQ(std::size(first_array).get(), 0);
		auto const second_array = array.get<1>();
		REQUIRE_EQ(std::size(second_array).get(), 0);
	}

	array.reserve(size_type{5});

	{
		EXPECT_EQ(array.size(), size_type{});
		EXPECT_EQ(array.capacity(), size_type{5});
		auto const first_array = array.get<0>();
		EXPECT_EQ(std::size(first_array).get(), 0);
		auto const second_array = array.get<1>();
		EXPECT_EQ(std::size(second_array).get(), 0);
	}
}

TESTCASE(terraformer_multi_array_push_back)
{
	terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>
	> array;

	for(int k = 0; k != 16; ++k)
	{
		if(static_cast<size_t>(k) == array.capacity().get())
		{
			no_default_constructible_type<int>::expect_move_ctor(array.capacity().get());
			no_default_constructible_type<double>::expect_move_ctor(array.capacity().get());
		}

		no_default_constructible_type<int>::expect_ctor();
		no_default_constructible_type<double>::expect_ctor();
		no_default_constructible_type<int>::expect_move_ctor();
		no_default_constructible_type<double>::expect_move_ctor();

		array.push_back(
			no_default_constructible_type<int>{k},
			no_default_constructible_type<double>{0.5*static_cast<double>(k)}
		);
	}

	auto const first_array = array.get<0>();
	auto const second_array = array.get<1>();

	for(auto k = array.first_element_index(); k != std::size(array); ++k)
	{
		EXPECT_EQ(static_cast<size_t>(static_cast<holder<int>>(first_array[k]).value()), k.get());
		EXPECT_EQ(static_cast<holder<double>>(second_array[k]).value(), 0.5*static_cast<double>(k.get()));
	}

	using size_type = terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>
	>::size_type;
	EXPECT_EQ(std::size(array), size_type{16});
}

TESTCASE(terraformer_multi_array_resize_grow_and_shrink)
{
	terraformer::multi_array<
		default_constructible_type<int>,
		default_constructible_type<double>
	> array;

	for(int k = 0; k != 4; ++k)
	{
		default_constructible_type<int>::expect_ctor();
		default_constructible_type<int>::expect_move_ctor();
		default_constructible_type<double>::expect_ctor();
		default_constructible_type<double>::expect_move_ctor();
		array.push_back(
			default_constructible_type<int>{k},
			default_constructible_type<double>{static_cast<double>(k)/2.0}
		);
	}

	default_constructible_type<int>::expect_move_ctor(4);
	default_constructible_type<double>::expect_move_ctor(4);
	default_constructible_type<int>::expect_ctor(12);
	default_constructible_type<double>::expect_ctor(12);

	using size_type = decltype(array)::size_type;
	array.resize(size_type{16});

	EXPECT_EQ(array.capacity().get(), 16);
	EXPECT_EQ(array.size().get(), 16);
	auto const first_array = array.get<0>();
	auto const second_array = array.get<1>();
	for(auto k = array.first_element_index() + 4; k != std::size(array); ++k)
	{
		EXPECT_EQ(static_cast<size_t>(static_cast<holder<int>>(first_array[k]).value()), 0);
		EXPECT_EQ(static_cast<holder<double>>(second_array[k]).value(), 0.0);
	}

	array.resize(size_type{8});
	EXPECT_EQ(array.size().get(), 8);
}

TESTCASE(terraformer_multi_array_move)
{
	terraformer::multi_array<
		default_constructible_type<int>,
		default_constructible_type<double>
	> array;

	default_constructible_type<int>::expect_ctor(4);
	default_constructible_type<double>::expect_ctor(4);
	array.push_back(1, 0.5);
	array.push_back(2, 1.0);
	array.push_back(3, 1.5);
	array.push_back(4, 2.0);

	auto const first_array_old = array.get<0>();
	auto const second_array_old = array.get<1>();

	auto other = std::move(array);
	EXPECT_EQ(std::size(array).get(), 0);
	EXPECT_EQ(std::size(other).get(), 4);

	auto const first_array_new = other.get<0>();
	auto const second_array_new = other.get<1>();
	EXPECT_EQ(std::data(first_array_new), std::data(first_array_old));
	EXPECT_EQ(std::data(second_array_new), std::data(second_array_old));

	auto const first_array_old_after_move = array.get<0>();
	auto const second_array_old_after_move = array.get<1>();
	EXPECT_EQ(std::data(first_array_old_after_move), nullptr);
	EXPECT_EQ(std::data(second_array_old_after_move), nullptr);
}

TESTCASE(terraformer_multi_array_copy)
{
	terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>
	> array;

	no_default_constructible_type<int>::expect_ctor(4);
	no_default_constructible_type<double>::expect_ctor(4);
	array.push_back(1, 0.5);
	array.push_back(2, 1.0);
	array.push_back(3, 1.5);
	array.push_back(4, 2.0);

	auto const first_array_old = array.get<0>();
	auto const second_array_old = array.get<1>();

	no_default_constructible_type<int>::expect_copy_ctor(4);
	no_default_constructible_type<double>::expect_copy_ctor(4);
	auto const other = array;

	EXPECT_EQ(std::size(array).get(), 4);
	EXPECT_EQ(std::size(other).get(), 4);

	auto const first_array_new = other.get<0>();
	auto const second_array_new = other.get<1>();
	EXPECT_NE(std::data(first_array_new), std::data(first_array_old));
	EXPECT_NE(std::data(second_array_new), std::data(second_array_old));

	auto const first_array_old_after_move = array.get<0>();
	auto const second_array_old_after_move = array.get<1>();
	EXPECT_EQ(std::data(first_array_old_after_move), std::data(first_array_old));
	EXPECT_EQ(std::data(second_array_old_after_move), std::data(second_array_old));
}

TESTCASE(terraformer_multi_array_move_assign)
{
	using my_array_type = terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>
	>;

	my_array_type array;

	no_default_constructible_type<int>::expect_ctor(4);
	no_default_constructible_type<double>::expect_ctor(4);
	array.push_back(1, 0.5);
	array.push_back(2, 1.0);
	array.push_back(3, 1.5);
	array.push_back(4, 2.0);

	my_array_type other;
	no_default_constructible_type<int>::expect_ctor(3);
	no_default_constructible_type<double>::expect_ctor(3);
	other.push_back(5, 2.5);
	other.push_back(6, 3.0);
	other.push_back(7, 3.5);
	auto const first_array_old = array.get<0>();
	auto const second_array_old = array.get<1>();
	auto const first_array_other_old = other.get<0>();
	auto const second_array_other_old = other.get<1>();

	array = std::move(other);

	auto const first_array_new = array.get<0>();
	auto const second_array_new = array.get<1>();
	auto const first_array_other_new = other.get<0>();
	auto const second_array_other_new = other.get<1>();

	EXPECT_NE(std::data(first_array_new), std::data(first_array_old));
	EXPECT_NE(std::data(second_array_new), std::data(second_array_old));
	EXPECT_EQ(std::data(first_array_new), std::data(first_array_other_old));
	EXPECT_EQ(std::data(second_array_new), std::data(second_array_other_old));
	EXPECT_EQ(std::data(first_array_other_new), nullptr);
	EXPECT_EQ(std::data(second_array_other_new), nullptr);
	EXPECT_EQ(std::size(array).get(), 3);
	EXPECT_EQ(std::size(other).get(), 0);
}

TESTCASE(terraformer_multi_array_insert_at_begin)
{
	using my_array_type = terraformer::multi_array<
		no_default_constructible_type<size_t>,
		no_default_constructible_type<double>
	>;

	my_array_type array;
	array.reserve(my_array_type::size_type{16});
	for(size_t k = 0; k != 16; ++k)
	{
		no_default_constructible_type<size_t>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		if(k != 0)
		{
			no_default_constructible_type<double>::expect_move_assign(k - 1);
			no_default_constructible_type<size_t>::expect_move_assign(k - 1);
			no_default_constructible_type<double>::expect_move_ctor(1);
			no_default_constructible_type<size_t>::expect_move_ctor(1);
		}
		array.insert(my_array_type::index_type{0}, k, 0.5*static_cast<double>(k));
	}

	EXPECT_EQ(std::size(array).get(), 16);

	auto const ints = array.get<0>();
	auto const doubles = array.get<1>();
	for(auto k = array.first_element_index(); k != std::size(array); ++k)
	{
		auto const expected_value = std::size(array).get() - k.get() - 1;
		EXPECT_EQ(static_cast<holder<size_t>>(ints[k]).value(), expected_value);
		EXPECT_EQ(static_cast<holder<double>>(doubles[k]).value(), 0.5f*static_cast<double>(expected_value));
	}
}

TESTCASE(terraformer_multi_array_insert_at_end)
{
	using my_array_type = terraformer::multi_array<
		no_default_constructible_type<size_t>,
		no_default_constructible_type<double>
	>;

	my_array_type array;
	array.reserve(my_array_type::size_type{16});
	for(size_t k = 0; k != 16; ++k)
	{
		no_default_constructible_type<size_t>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		array.insert(my_array_type::index_type{k}, k, 0.5*static_cast<double>(k));
	}

	auto const ints = array.get<0>();
	auto const doubles = array.get<1>();
	for(auto k = array.first_element_index(); k != std::size(array); ++k)
	{
		auto const expected_value = k.get();
		EXPECT_EQ(static_cast<holder<size_t>>(ints[k]).value(), expected_value);
		EXPECT_EQ(static_cast<holder<double>>(doubles[k]).value(), 0.5f*static_cast<double>(expected_value));
	}
}

TESTCASE(terraformer_multi_array_insert_somewhere)
{
	using my_array_type = terraformer::multi_array<
		no_default_constructible_type<size_t>,
		no_default_constructible_type<double>
	>;

	my_array_type array;
	array.reserve(my_array_type::size_type{24});
	for(size_t k = 0; k != 8; ++k)
	{
		no_default_constructible_type<size_t>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		array.push_back(k, 0.5*static_cast<double>(k));
	}

	for(size_t k = 0; k != 8; ++k)
	{
		no_default_constructible_type<size_t>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		array.push_back(k + 16, 0.5*static_cast<double>(k + 16));
	}

	for(size_t k = 0; k != 8; ++k)
	{
		no_default_constructible_type<size_t>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		no_default_constructible_type<double>::expect_move_assign(7);
		no_default_constructible_type<size_t>::expect_move_assign(7);
		no_default_constructible_type<double>::expect_move_ctor(1);
		no_default_constructible_type<size_t>::expect_move_ctor(1);
		array.insert(my_array_type::index_type{k + 8}, k + 8, 0.5*static_cast<double>(k + 8));
	}

	auto const ints = array.get<0>();
	auto const doubles = array.get<1>();
	for(auto k = array.first_element_index(); k != std::size(array); ++k)
	{
		auto const expected_value = k.get();
		EXPECT_EQ(static_cast<holder<size_t>>(ints[k]).value(), expected_value);
		EXPECT_EQ(static_cast<holder<double>>(doubles[k]).value(), 0.5f*static_cast<double>(expected_value));
	}
}

TESTCASE(terraformer_multi_array_insert_somewhere_with_one_reservation)
{
	using my_array_type = terraformer::multi_array<
		no_default_constructible_type<size_t>,
		no_default_constructible_type<double>
	>;

	my_array_type array;
	for(size_t k = 0; k != 8; ++k)
	{
		no_default_constructible_type<size_t>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		array.push_back(k, 0.5*static_cast<double>(k));
	}

	no_default_constructible_type<double>::expect_move_ctor(8);
	no_default_constructible_type<size_t>::expect_move_ctor(8);

	for(size_t k = 0; k != 8; ++k)
	{
		no_default_constructible_type<size_t>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		array.push_back(k + 16, 0.5*static_cast<double>(k + 16));
	}

	no_default_constructible_type<double>::expect_move_ctor(16);
	no_default_constructible_type<size_t>::expect_move_ctor(16);

	for(size_t k = 0; k != 8; ++k)
	{
		no_default_constructible_type<size_t>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		no_default_constructible_type<double>::expect_move_assign(7);
		no_default_constructible_type<size_t>::expect_move_assign(7);
		no_default_constructible_type<double>::expect_move_ctor(1);
		no_default_constructible_type<size_t>::expect_move_ctor(1);
		array.insert(my_array_type::index_type{k + 8}, k + 8, 0.5*static_cast<double>(k + 8));
	}

	auto const ints = array.get<0>();
	auto const doubles = array.get<1>();
	for(auto k = array.first_element_index(); k != std::size(array); ++k)
	{
		auto const expected_value = k.get();
		EXPECT_EQ(static_cast<holder<size_t>>(ints[k]).value(), expected_value);
		EXPECT_EQ(static_cast<holder<double>>(doubles[k]).value(), 0.5f*static_cast<double>(expected_value));
	}
}

TESTCASE(terraformer_multi_array_partial_assign_value)
{
	using my_array_type = terraformer::multi_array<
		no_default_constructible_type<int>,
		no_default_constructible_type<double>,
		no_default_constructible_type<std::string>
	>;

	{
		my_array_type array;
		no_default_constructible_type<int>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		no_default_constructible_type<std::string>::expect_ctor(1);
		array.push_back(1, 0.5, "A long string that should trigger malloc");
		no_default_constructible_type<int>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		no_default_constructible_type<std::string>::expect_ctor(1);
		array.push_back(2, 1.5, "Kaka");
		no_default_constructible_type<int>::expect_ctor(1);
		no_default_constructible_type<double>::expect_ctor(1);
		no_default_constructible_type<std::string>::expect_ctor(1);
		array.push_back(3, 3.5, "Bulle");

		no_default_constructible_type<double>::expect_ctor(1);
		no_default_constructible_type<std::string>::expect_ctor(1);
		no_default_constructible_type<double>::expect_move_assign(1);
		no_default_constructible_type<std::string>::expect_move_assign(1);
		array.assign<1>(my_array_type::index_type{0}, 8.0, "Other string that should trigger malloc");

		{
			EXPECT_EQ(static_cast<holder<int>>(array.get<0>()[my_array_type::index_type{0}]).value(), 1);
			EXPECT_EQ(static_cast<holder<double>>(array.get<1>()[my_array_type::index_type{0}]).value(), 8.0);
			auto const str = static_cast<holder<std::string>>(array.get<2>()[my_array_type::index_type{0}]).value();
			EXPECT_EQ(str, "Other string that should trigger malloc");
		}

		{
			EXPECT_EQ(static_cast<holder<int>>(array.get<0>()[my_array_type::index_type{1}]).value(), 2);
			EXPECT_EQ(static_cast<holder<double>>(array.get<1>()[my_array_type::index_type{1}]).value(), 1.5);
			auto const str = static_cast<holder<std::string>>(array.get<2>()[my_array_type::index_type{1}]).value();
			EXPECT_EQ(str, "Kaka");
		}

		{
			EXPECT_EQ(static_cast<holder<int>>(array.get<0>()[my_array_type::index_type{2}]).value(), 3);
			EXPECT_EQ(static_cast<holder<double>>(array.get<1>()[my_array_type::index_type{2}]).value(), 3.5);
			auto const str = static_cast<holder<std::string>>(array.get<2>()[my_array_type::index_type{2}]).value();
			EXPECT_EQ(str, "Bulle");
		}
	}
}

TESTCASE(terraformer_multi_array_get_attribs)
{
	using my_array_type = terraformer::multi_array<int, double, std::string>;

	my_array_type array;
	array.push_back(1, 0.5, "A long string that should trigger malloc");
	array.push_back(2, 1.5, "Kaka");
	array.push_back(3, 3.5, "Bulle");

	auto span = array.attributes();
	EXPECT_EQ(std::size(span), std::size(array));
	EXPECT_EQ(std::begin(span.get<0>()), std::begin(array.get<0>()));
	EXPECT_EQ(std::begin(span.get<1>()), std::begin(array.get<1>()));
	EXPECT_EQ(std::begin(span.get<2>()), std::begin(array.get<2>()));

	auto const index_to_change = span.first_element_index() + 1;
	span.assign<0>(index_to_change, 4, 4.5, "A new value");
	EXPECT_EQ(span.get<0>()[index_to_change], 4);
	EXPECT_EQ(span.get<1>()[index_to_change], 4.5);
	EXPECT_EQ(span.get<2>()[index_to_change], "A new value");
	EXPECT_EQ(array.get<0>()[index_to_change], 4);
	EXPECT_EQ(array.get<1>()[index_to_change], 4.5);
	EXPECT_EQ(array.get<2>()[index_to_change], "A new value");
}

TESTCASE(terraformer_multi_array_get_attribs_const)
{
	using my_array_type = terraformer::multi_array<int, double, std::string>;

	my_array_type array;
	array.push_back(1, 0.5, "A long string that should trigger malloc");
	array.push_back(2, 1.5, "Kaka");
	array.push_back(3, 3.5, "Bulle");

	auto span = std::as_const(array).attributes();
	EXPECT_EQ(std::size(span), std::size(array));
	EXPECT_EQ(std::begin(span.get<0>()), std::begin(array.get<0>()));
	EXPECT_EQ(std::begin(span.get<1>()), std::begin(array.get<1>()));
	EXPECT_EQ(std::begin(span.get<2>()), std::begin(array.get<2>()));

	static_assert(
		std::is_same_v<
			terraformer::compatible_multi_array_t<decltype(span)>,
			my_array_type
		>
	);
}