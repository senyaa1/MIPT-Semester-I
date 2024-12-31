#include "logger.h"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

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
			printf(RED "Error during testing! Verify test files!\n" RESET);
			break;
	}
}

void print_available_tests(tested_program_t* tp)
{
	printf(BLU "Available tests (%ld): \n" RESET, tp->test_cnt);
	
	for(int i = 0; i < tp->test_cnt; i++)
	{
		printf(YEL "\t%s\n" RESET, tp->tests[i]);
	}
}
