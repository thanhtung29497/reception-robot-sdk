#ifndef LAPJV_H
#define LAPJV_H

#define RECTANGULAR_LSAP_INFEASIBLE -1
#define RECTANGULAR_LSAP_INVALID -2

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
/**
 * @brief Solve Linear assignment problem
 * 
 * @param n_rows number of rows of cost matrix
 * @param n_cols number of columns of cost matrix
 * @param cost_matrix [in] cost matrix
 * @param maxmimize true if we want to maximize, false if we want to minimize
 * @param a [out] row index of solution
 * @param b [out] column index of solution
 * @return int 
 */
int solve_lap(intptr_t n_rows, intptr_t n_cols, double *cost_matrix, bool maxmimize, int64_t* a, int64_t* b);

#ifdef __cplusplus
}
#endif
#endif
