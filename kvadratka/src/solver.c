/** @file solver.c
 *  @brief File with a function to solve quadratic equations
 *
 *  Contains functions to solve quadratic equations with specified accuracy
 */

#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#include "solver.h"
#include "comparators.h"
#include "basedssert.h"

void solve_quadratic(quadratic_eq_t* eq)
{
	basedssert(eq);
	basedssert(isfinite(eq->a));
	basedssert(isfinite(eq->b));
	basedssert(isfinite(eq->c));

	if(d_eq(eq->a, 0) && d_eq(eq->b, 0))
	{
		eq->cnt = (d_eq(eq->c, 0)) ? INF_ROOTS: NO_ROOTS;
		return;
	}

	if(d_eq(eq->a, 0))
	{
		eq->x1 = -eq->c / eq->b;
		eq->cnt = ONE_ROOT;
		return;
	}

	double d = (eq->b * eq->b) - 4 * eq->a * eq->c;

	if (d < 0)
	{
		eq->cnt = NO_ROOTS;
		return;
	}
	eq->x1 = (-eq->b + sqrt(d)) / (2 * eq->a);
	eq->x2 = (-eq->b - sqrt(d)) / (2 * eq->a);
	
	eq->cnt = (d_eq(d, 0)) ? ONE_ROOT : TWO_ROOTS;
}
