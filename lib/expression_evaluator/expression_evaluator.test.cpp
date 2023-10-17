//@	{"target":{"name":"expression_evaluator.test"}}

#include "./expression_evaluator.hpp"

#include <testfwk/testfwk.hpp>

TESTCASE(terraformer_expression_evaluator_expression_without_whitespace)
{
	terraformer::expression_evaluator::evaluate("  foo ( 1,  3, bar( kaka(5)) , 7)");

	terraformer::expression_evaluator::evaluate("  foo ( 1 ,  3, bar ( kaka(5)) , 7)");
}
