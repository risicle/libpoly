/*
 * univariate_polynomial_factorization.h
 *
 *  Created on: Nov 8, 2013
 *      Author: dejan
 */

#pragma once

#include <stdio.h>

#include "upolynomial/upolynomial.h"

/**
 * Factors the given polynomial into square-free factors. Polynomial f should be
 * monic if in Z_p, or primitive if in Z.
 */
upolynomial_factors_t* upolynomial_factor_square_free(const upolynomial_t* f);

/**
 * Factors the given polynomial into a distinct degree factorization.
 * Polynomial f should in Z_p, square-free, and monic.
 *
 * The output of the function is a factorization where each factor is associated
 * with a distinct degree of each of its sub-factors.
 */
upolynomial_factors_t* upolynomial_factor_distinct_degree(const upolynomial_t* f);

/**
 * Factors the given polynomial using the algorithm of Berlekamp. The algorithm
 * assumes that p is in a ring Z_p for some prime p.
 */
upolynomial_factors_t* upolynomial_factor_Zp(const upolynomial_t* f);

/**
 * Factors the given polynomial using Hansel lifting.
 */
upolynomial_factors_t* upolynomial_factor_Z(const upolynomial_t* f);