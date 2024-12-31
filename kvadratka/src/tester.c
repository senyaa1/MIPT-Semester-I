#include <stdio.h>
#include <stdbool.h>
#include "tester.h"
#include "solver.h"
#include "comparators.h"
#include "io.h"


void print_test_result(const char* test_name, test_result_t result)
{
	printf("%s: ", test_name);
	switch(result)
	{
		case PASSED:
			printf(GRN "PASSED!\n" RESET);
			break;
		case FAILED:
			printf(RED "FAILED!\n" RESET);
			break;
		default:
		case TESTING_ERROR:
			fprintf(stderr, RED "Error during testing! Verify test files!\n" RESET);
			break;
	}
}

test_result_t test_sq(quadratic_eq_t* eq, quadratic_eq_t* expected_output)
{
	solve_quadratic(eq);
	return (eq_cmp(eq, expected_output) ? PASSED : FAILED);
}

void run_all_sq_tests()
{
	quadratic_eq_t eq, expected;
	eq = (quadratic_eq_t){0, 0, 0, 0, 0, NOT_CALCULATED};
	expected = (quadratic_eq_t){0, 0, 0, 0, 0, INF_ROOTS};
	print_test_result("inf_roots", test_sq(&eq, &expected));

	eq = (quadratic_eq_t){0, 0, 5, 0, 0, NOT_CALCULATED};
	expected = (quadratic_eq_t){0, 0, 5, 0, 0, NO_ROOTS};
	print_test_result("no_roots", test_sq(&eq, &expected));

	eq = (quadratic_eq_t){0, 1, 2, 0, 0, NOT_CALCULATED};
	expected = (quadratic_eq_t){0, 1, 2, -2, 0, ONE_ROOT};
	print_test_result("one_root", test_sq(&eq, &expected));

	eq = (quadratic_eq_t){1, 5, 6, 0, 0, NOT_CALCULATED};
	expected = (quadratic_eq_t){1, 5, 6, -2, -3, TWO_ROOTS};
	print_test_result("two_roots1", test_sq(&eq, &expected));

	eq = (quadratic_eq_t){2, 0, -32, 0, 0, NOT_CALCULATED};
	expected = (quadratic_eq_t){2, 0, -32, 4, -4, TWO_ROOTS};
	print_test_result("two_roots2", test_sq(&eq, &expected));
}
