#pragma once
#include <stdio.h>
#include <stdbool.h>

#define MAX_PATH_LEN 1024
#define MAX_OUTPUT_SZ 4096
#define MAX_INPUT_SZ 4096
#define MAX_TEST_CNT 2048

typedef struct tested_program
{
	char* tests_path;
	char* executable_path;

	char** tests;
	size_t test_cnt;
} tested_program_t;

typedef enum test_result
{
	PASSED = 1,
	FAILED = 2,
	TESTING_ERROR = 4
} test_result_t;

bool initialize(tested_program_t *tp, char* program_path, char* tests_directory);

test_result_t run_test(tested_program_t* tp, const char* test_name);

void run_all_tests(tested_program_t* tp);

void destroy_tp(tested_program_t* tp);

test_result_t run_and_print(const char* test_name, tested_program_t* tp);
