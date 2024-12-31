#pragma once

#include "solver.h"

typedef enum test_result
{
	PASSED = 1,
	FAILED = 2,
	TESTING_ERROR = 4
} test_result_t;


void print_test_result(const char* test_name, test_result_t result);
test_result_t test_sq(quadratic_eq_t* input, quadratic_eq_t* expected_output);
void run_all_sq_tests();
