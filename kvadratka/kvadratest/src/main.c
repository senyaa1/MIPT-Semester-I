#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include "runner.h"
#include "logger.h"


int main(int argc, char** argv)
{
	tested_program_t tp;
	if(!initialize(&tp, "../bin/kvadratka", "tests"))
	{
		printf("can't initialize testing framework!\n");
		return 1;
	}

	print_available_tests(&tp);

	/*test_result_t res = run_test(&tp, "abc");*/
	/*print_test_result("abc", res);*/
	
	run_all_tests(&tp);

	destroy_tp(&tp);

	return 0;
}
