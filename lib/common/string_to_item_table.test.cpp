//@	{
//@		"target":{
//@			"name":"string_to_item_table.test",
//@			"dependencies": [{"ref":"langmorph", "origin":"pkg-config"}]
//@		}
//@	}

#include "./string_to_item_table.hpp"
#include "./rng.hpp"

#include <langmorph/word_factory.hpp>

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_string_to_item_table_direct_insert_and_find)
{
	terraformer::string_to_item_table<int, true> values{
		"key 1",
		13432
	};

	// Finding first item
	{
		auto const res = values.at_ptr("key 1");
		REQUIRE_NE(res, nullptr);
		EXPECT_EQ(*res, 13432);
		EXPECT_EQ(res, &values.first_element_value());
	}

	// Try to replace first item with `insert`
	{
		auto const res = values.insert("key 1", 545);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 1");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 13432);
	}

	// Replace first item with insert_or_assign
	{
		auto const res = values.insert_or_assign("key 1", 545);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 1");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 545);
	}

	// Try to find a non-existing item
	{
		auto const res = values.at_ptr("key 2");
		EXPECT_EQ(res, nullptr);
	}

	// Insert a new item with insert
	{
		EXPECT_EQ(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert("key 3", 456);
		EXPECT_EQ(res.second, true);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 456);
	}

	// Try to replace existing item with insert
	{
		EXPECT_NE(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert("key 3", 777);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 456);
	}

	// Insert new item with insert_or_assign
	{
		EXPECT_EQ(values.at_ptr("key 2"), nullptr);
		auto const res = values.insert_or_assign("key 2", 634);
		EXPECT_EQ(res.second, true);
		auto const find_res = values.at_ptr("key 2");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 634);
	}

	// Replace existing item (not first one) with insert_or_assign
	{
		EXPECT_NE(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert_or_assign("key 3", 777);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 777);
	}
}

TESTCASE(terraformer_string_to_item_table_insert_and_find)
{
	terraformer::string_to_item_table<int> values;

	// Table is initially empty
	{
		auto const res = values.at_ptr("key 1");
		EXPECT_EQ(res, nullptr);
	}

	// Insert a new item with insert
	{
		EXPECT_EQ(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert("key 3", 456);
		EXPECT_EQ(res.second, true);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 456);
	}

	// Try to replace existing value with insert
	{
		EXPECT_NE(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert("key 3", 777);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 456);
	}

	// Insert a new value with insert_or_assign
	{
		EXPECT_EQ(values.at_ptr("key 2"), nullptr);
		auto const res = values.insert_or_assign("key 2", 634);
		EXPECT_EQ(res.second, true);
		auto const find_res = values.at_ptr("key 2");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 634);
	}

	// Replace existing value with insert_or_assign
	{
		EXPECT_NE(values.at_ptr("key 3"), nullptr);
		auto const res = values.insert_or_assign("key 3", 777);
		EXPECT_EQ(res.second, false);
		auto const find_res = values.at_ptr("key 3");
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 777);
	}
}

TESTCASE(terraformer_string_to_item_table_hash_collision)
{
	terraformer::string_to_item_table<int> values;

	char const* key_1 = "8yn0iYCKYHlIj4-BwPqk";
	char const* key_2 = "GReLUrM4wMqfg9yzV3KQ";
	REQUIRE_EQ(terraformer::make_hash(key_1), terraformer::make_hash(key_2));

	{
		auto const res = values.insert(key_1, 456);
		EXPECT_EQ(res.second, true);
		auto const find_res = values.at_ptr(key_1);
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 456);
	}

	{
		auto const res = values.insert(key_2, 131);
		EXPECT_EQ(res.second, true);
		auto const find_res = values.at_ptr(key_2);
		REQUIRE_NE(find_res, nullptr);
		EXPECT_EQ(*find_res, 131);
	}

	{
		auto const find_res_1 = values.at_ptr(key_1);
		auto const find_res_2 = values.at_ptr(key_2);
		EXPECT_NE(find_res_1, find_res_2);
		REQUIRE_NE(find_res_1, nullptr);
		EXPECT_EQ(*find_res_1, 456);
		REQUIRE_NE(find_res_2, nullptr);
		EXPECT_EQ(*find_res_2, 131);
	}
}

TESTCASE(terraformer_string_to_item_table_random_lookup_perf)
{
	auto factory = create(
		std::type_identity<langmorph::word_factory>{},
		"testdata/shakespeare.wad64",
		"langmorph_data"
	);

	terraformer::random_generator rng;
	size_t dict_size = 1;

	while(dict_size <= 16384)
	{
		size_t lookup_count = 1024*1024*8;

		printf("%zu", dict_size);
		fflush(stdout);
		for(size_t length = 1; length != 4; ++length)
		{
			terraformer::string_to_item_table<size_t> values;
			std::chrono::duration<double> t_insert;
			std::vector<std::string> keys;
			{
				auto now = std::chrono::steady_clock::now();
				for(size_t k = 0; k != dict_size; ++k)
				{
					while(true)
					{
						std::string key;
						for(size_t elem_count = 0; elem_count != length; ++elem_count)
						{ key += std::string{"/"}.append(factory(rng)); }

						if(values.insert(key.c_str(), k ).second)
						{
							keys.push_back(std::move(key));
							break;
						}
					}
				}

				t_insert = std::chrono::duration<double>{std::chrono::steady_clock::now() - now}
					/static_cast<double>(dict_size);
			}

			std::chrono::duration<double> t_lookup;
			{
				auto now = std::chrono::steady_clock::now();
				std::uniform_int_distribution U{static_cast<size_t>(0), std::size(keys) - 1};
				for(size_t k = 0; k != lookup_count; ++k)
				{
					auto const index = U(rng);
					auto const find_res = values.at_ptr(keys[index].c_str());
					REQUIRE_NE(find_res, nullptr);
				}

				t_lookup = std::chrono::duration<double>{std::chrono::steady_clock::now() - now}
					/static_cast<double>(lookup_count);
			}
			printf(" %.8g", t_lookup.count());
			fflush(stdout);
		}
		printf("\n");
		fflush(stdout);
		dict_size <<= 1;
	}
}

TESTCASE(terraformer_string_to_item_table_flatmap_random_lookup_perf)
{
	auto factory = create(
		std::type_identity<langmorph::word_factory>{},
		"testdata/shakespeare.wad64",
		"langmorph_data"
	);

	terraformer::random_generator rng;
	size_t dict_size = 1;

	while(dict_size <= 16384)
	{
		size_t lookup_count = 1024*1024*8;

		printf("%zu", dict_size);
		fflush(stdout);
		for(size_t length = 1; length != 4; ++length)
		{
			terraformer::flat_map<std::less<>, std::string, size_t> values;
			std::chrono::duration<double> t_insert;
			std::vector<std::string> keys;
			{
				auto now = std::chrono::steady_clock::now();
				for(size_t k = 0; k != dict_size; ++k)
				{
					while(true)
					{
						std::string key;
						for(size_t elem_count = 0; elem_count != length; ++elem_count)
						{ key += std::string{"/"}.append(factory(rng)); }

						if(values.insert(key, k ).second)
						{
							keys.push_back(std::move(key));
							break;
						}
					}
				}

				t_insert = std::chrono::duration<double>{std::chrono::steady_clock::now() - now}
					/static_cast<double>(dict_size);
			}

			std::chrono::duration<double> t_lookup;
			{
				auto now = std::chrono::steady_clock::now();
				std::uniform_int_distribution U{static_cast<size_t>(0), std::size(keys) - 1};
				for(size_t k = 0; k != lookup_count; ++k)
				{
					auto const index = U(rng);
					auto const find_res = values.find(keys[index]);
					REQUIRE_NE(find_res, (terraformer::flat_map<std::less<>, std::string, size_t>::npos));
				}

				t_lookup = std::chrono::duration<double>{std::chrono::steady_clock::now() - now}
					/static_cast<double>(lookup_count);
			}
			printf(" %.8g", t_lookup.count());
			fflush(stdout);
		}
		printf("\n");
		fflush(stdout);
		dict_size <<= 1;
	}
}