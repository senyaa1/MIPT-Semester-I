#include <stdbool.h>
#include "solver.h"

// LTO не работает, задаём инлайн функции в каждом файле
static const double eps = 0.00001;

inline static bool d_eq(double a, double b)
{
	return fabs(a - b) <= eps;
}

inline static bool eq_cmp(quadratic_eq_t* a, quadratic_eq_t* b)
{
	bool res = true;

	res &= (a->cnt == b->cnt);
	res &= d_eq(a->a, b->a);
	res &= d_eq(a->b, b->b);
	res &= d_eq(a->c, b->c);
	res &= d_eq(a->x1, b->x1);
	res &= d_eq(a->x2, b->x2);

	return res;
}
