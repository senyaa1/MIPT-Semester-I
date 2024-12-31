#pragma once

#include <stdio.h>
#include "runner.h"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

void print_test_result(const char* test_name, test_result_t result);
void print_available_tests(tested_program_t* tp);
