/** @file io.h
 *  @brief File with input/output functions
 *
 *  Contains functions to handle input and output of kvadratka 
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "solver.h"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define UNDERLINE "\e[4m"
#define RESET "\x1B[0m"

/** @brief Prints coefficients to stdout
 *  @param eq Quadratic equation
 *  @return Exit code
 */
int pretty_print_roots(quadratic_eq_t* eq);
/** @brief Reads coefficients from the stdin
 *  @param eq Quadratic equation
 *  @return void
 */
void read_coeffs(quadratic_eq_t* eq);
/** @brief Reads coefficients from the argv array
 *  @param eq Quadratic equation
 *  @return true if argv had correct data, false otherwise
 */
bool read_coeffs_from_argv(quadratic_eq_t* eq, char** argv);
