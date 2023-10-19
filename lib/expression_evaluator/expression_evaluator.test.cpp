//@	{"target":{"name":"expression_evaluator.test"}}

#include "./expression_evaluator.hpp"

#include <testfwk/testfwk.hpp>

namespace
{
	struct parser_context
	{
		std::string command_name;
		std::vector<std::string> args;
	};

	struct context_evalutor
	{
		using argument_type = std::string;

		auto create_context(std::string&& command_name)
		{
			return parser_context{std::move(command_name), std::vector<std::string>{}};
		}


		std::string evaluate(parser_context const& ctxt)
		{
			std::string ret{"["};
			ret.append(ctxt.command_name).append("(");
			for(size_t k = 0; k != std::size(ctxt.args); ++k)
			{
				ret.append(std::string{"+"}.append(ctxt.args[k]));
			}
			ret.append(")]");
			return ret;
		}
	};

	struct string_converter
	{
		std::string convert(std::string const& buffer)
		{
			return buffer + "_conv";
		}
	};
}


TESTCASE(terraformer_expression_evaluator_parse_value_only)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "foobar_conv")
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_value_only_leading_whitespace)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"    foobar"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "foobar_conv")
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_value_only_trailing_whitespace)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar    "};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "foobar_conv")
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_start_with_delimiter)
{
	context_evalutor eval{};
	string_converter converter{};
	{
		std::string_view str{"(foobar    "};
		try
		{
			auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
			abort();
		}
		catch(terraformer::input_error const& err)
		{ EXPECT_EQ(err.what(), std::string_view{"Empty command name"}); }
	}

	{
		std::string_view str{"     ,foobar    "};
		try
		{
			auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
			abort();
		}
		catch(terraformer::input_error const& err)
		{ EXPECT_EQ(err.what(), std::string_view{"Empty command name"}); }
	}
}

TESTCASE(terraformer_expression_evaluator_parse_wrong_delimiter_after_initial_command_name)
{
	context_evalutor eval{};
	string_converter converter{};
	{
		std::string_view str{"foobar,"};
		try
		{
			auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
			abort();
		}
		catch(terraformer::input_error const& err)
		{ EXPECT_EQ(err.what(), std::string_view{"A command name may not contain `,`"}); }
	}

	{
		std::string_view str{"foobar)"};
		try
		{
			auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
			abort();
		}
		catch(terraformer::input_error const& err)
		{ EXPECT_EQ(err.what(), std::string_view{"A command name may not contain `)`"}); }
	}
}

TESTCASE(terraformer_expression_evaluator_parse_bad_char_after_initial_command_name)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar kaka"};
	try
	{
		auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"Unexpected character after command name"}); }
}

TESTCASE(terraformer_expression_evaluator_parse_wrong_delimiter_before_list_item)
{
	context_evalutor eval{};
	string_converter converter{};
	{
		std::string_view str{"foobar(,"};
		try
		{
			auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
			abort();
		}
		catch(terraformer::input_error const& err)
		{ EXPECT_EQ(err.what(), std::string_view{"A list item may not contain `,`"}); }
	}

	{
		std::string_view str{"foobar(("};
		try
		{
			auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
			abort();
		}
		catch(terraformer::input_error const& err)
		{ EXPECT_EQ(err.what(), std::string_view{"A list item may not contain `(`"}); }
	}
}

TESTCASE(terraformer_expression_evaluator_parse_command_with_no_arguments)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar()"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "[foobar()]")
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_command_whitespace_after_command_name_with_no_arguments)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar   ()"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "[foobar()]")
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_two_nested_commands_no_arguments)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar(  kaka())"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "[foobar(+[kaka()])]")
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_command_with_some_args)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar(arg1, arg2 ,arg3 () , arg4)"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "[foobar(+arg1_conv+arg2_conv+[arg3()]+arg4_conv)]");
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_command_with_some_args_whitespace_before_end)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar(arg1, arg2 ,arg3 () , arg4  )"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "[foobar(+arg1_conv+arg2_conv+[arg3()]+arg4_conv)]");
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_command_with_some_args_junk_after_list_item)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar(arg1, arg2 arg3)"};
	try
	{
		auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"Unexpected character at end of list item"}); }
}

TESTCASE(terraformer_expression_evaluator_parse_command_with_some_args_junk_after_command)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar(arg1, arg2() arg3)"};
	try
	{
		auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
		abort();
	}
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"Unexpected character at end of command"}); }
}

TESTCASE(terraformer_expression_evaluator_parse_nested_command_with_whitespace_after_inner_arg)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar(arg1, arg2(foo ))"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "[foobar(+arg1_conv+[arg2(+foo_conv)])]");
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_nested_command_command_left_after_command)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar(arg1, arg2(arg3(foo )))"};
	auto const res = terraformer::expression_evaluator::parse(str, eval, converter);
	EXPECT_EQ(res.result, "[foobar(+arg1_conv+[arg2(+[arg3(+foo_conv)])])]");
	EXPECT_EQ(res.expression_end, std::end(str));
}

TESTCASE(terraformer_expression_evaluator_parse_unterminated_command)
{
	context_evalutor eval{};
	string_converter converter{};
	std::string_view str{"foobar(arg1, arg2(arg3(foo ))"};
	try
	{ auto const res = terraformer::expression_evaluator::parse(str, eval, converter); }
	catch(terraformer::input_error const& err)
	{ EXPECT_EQ(err.what(), std::string_view{"Unterminated command"}); }
}