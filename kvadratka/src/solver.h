/** @file solver.h
 *  @brief File with a function to solve quadratic equations
 *
 *  Contains functions to solve quadratic equations with specified accuracy
 */
#pragma once

#include <stdint.h>
#include <math.h>
#include <assert.h>

typedef enum sq_root_cnt {
	NO_ROOTS = 0,
	ONE_ROOT = 1,
	TWO_ROOTS = 2,
	INF_ROOTS = 123,
	NOT_CALCULATED = 133,
} sq_root_cnt_t;

typedef double coeff_t;
typedef double root_t;

typedef struct quadratic_eq {
	coeff_t a;
	coeff_t b;
	coeff_t c;

	root_t x1;
	root_t x2;

	sq_root_cnt_t cnt;
} quadratic_eq_t;


/** @brief Solves quadratic equation
*
*  @param eq Quadratic equation to be solved
*  @return void.
*/

void solve_quadratic(quadratic_eq_t* eq);
