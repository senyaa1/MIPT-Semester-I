/** @file main.c
 *  @brief Main file
 *
 *  Contains main function which runs kvadratka
 */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "solver.h"
#include "tester.h"
#include "debugger.h"
#include "io.h"

/** @brief Kvadratka entry
 *
 * This is the program entrypoint
 *
 * @return Should not return
 */
int main(int argc, char** argv)
{
	setup_signals();

	// if(check_ptr((void*)0, "r"))
	// {
	// 	printf("able to read zero\n");
	// 
	// }
	// if(check_ptr((void*)0xbabe, "r"))
	// {
	// 	printf("able to read 0xbabe\n");
	// }
	//
	// if(check_ptr(argv, "r"))
	// {
	// 	printf("able to read argv\n");
	// }
	//
	// if(check_ptr(argv, "w"))
	// {
	// 	printf("able to write argv\n");
	// }
	//
	fflush(stdout);

	*(int*)(0) = 0;

	quadratic_eq_t eq = (quadratic_eq_t){0, 0, 0, 0, 0, NOT_CALCULATED};

	if(argc == 4 && read_coeffs_from_argv(&eq, argv)) 
	{
		solve_quadratic(&eq);
		return pretty_print_roots(&eq);
	}

	int opt;
	while ((opt = getopt(argc, argv, "ht")) != -1) {
		switch (opt) {
		        case 't': 
				run_all_sq_tests();
				return 0;
			case 'h':
		        default:
				if(argc == 4) break;
				fprintf(stderr, "Usage: %s [-t] (coefficients)\n", argv[0]);
				return 1;
	        }	
	}

	read_coeffs(&eq);
	solve_quadratic(&eq);
	return pretty_print_roots(&eq);
}

