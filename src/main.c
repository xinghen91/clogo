/*********************************************************************
* INCLUDES
*********************************************************************/
#include "clogo/clogo.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>


/*********************************************************************
* CONSTANTS
*********************************************************************/
//Fancy deferring macro for fancy preprocessor junk
#define CAT(a, ...) a ## __VA_ARGS__
#define _FN_MAX(x) CAT(MAX__, x)
#define FN_MAX _FN_MAX(FN)

//Maximum values of functions provided in this module
#define MAX__rosenbrock_2 (0.0)
#define MAX__sin_2 (0.9517936893872353)

//Function to test
#define FN rosenbrock_2


/*********************************************************************
* FUNCTIONS
*********************************************************************/

/***********************************************************
* rosenbrock_2
*
* 2D rosenbrock function - maps [0,1] to [-5,10].
***********************************************************/
double rosenbrock_2(
  double *i
) 
{
  double x = i[0], y = i[1];
  double min = -5.0;
  double max = 10.0;
  x = min + x * (max - min);
  y = min + y * (max - min);
  return -(100.0 * pow(y - x * x, 2.0) + pow(x * x - 1.0, 2.0));
} /* rosenbrock_2() */

/***********************************************************
* sin_helper
*
* Convenience function for calculating sin_X.
***********************************************************/
double sin_helper(
  double x
)
{
  return (sin(13.0 * x)*sin(27.0 * x) + 1.0) / 2;
} /* sin_helper() */

/***********************************************************
* sin_2
*
* 2D sin test function.
***********************************************************/
double sin_2(
  double *i
)
{
  double x = i[0], y = i[1];
  return sin_helper(x) * sin_helper(y);
} /* sin_2() */

/***********************************************************
* hmax
*
* Function that describes the maximum depth level to 
* consider given a certain number of function evaluations.
***********************************************************/
double hmax(
  int n                    //current number of function eval
)
{
  return sqrt((double)n);
} /* hmax() */

/***********************************************************
* logo_schedule
*
* w schedule for the LOGO algorithm.
***********************************************************/
int logo_schedule(
  const struct cl_state *state
)
{
  static const int w[] = {3, 4, 5, 6, 8, 30};
  //Find index of current w value
  int w_cnt = sizeof(w)/sizeof(w[0]);
  int j = -1;
  for (int i = 0; i < w_cnt; i++) {
    if (state->w == w[i]) {
      j = i;
      break;
    }
  }
  assert(j != -1);

  //Decide index of next w value
  int k = j;
  double new_best = state_best_value(state); 
  if (new_best > state->last_best_value) {
    k++;
  } else {
    k--;
  }

  //Clip index
  if (k < 0) k = 0;
  else if (k >= w_cnt) k = w_cnt-1;

  return w[k];
} /* logo_schedule() */

/***********************************************************
* soo_schedule
*
* w schedule for the SOO algorithm. Always 1.
***********************************************************/
int soo_schedule(const struct cl_state *state)
{
  (void)state; //Don't need to use the parameter if we 
               //always return the same value.
  return 1;
} /* soo_schedule() */

/***********************************************************
* display_result
*
* Print out debug info about a result structure.
***********************************************************/
void display_result(
  struct clogo_result *result
)
{
  printf("samples: %d\t error: %e\t point: %f/%f\n",
         result->samples, FN_MAX - result->value,
         result->point[0], result->point[1]);
} /* display_result() */

/***********************************************************
* test_soo
*
* Run the optimization using SOO-like settings.
***********************************************************/
struct clogo_options test_soo()
{
  struct clogo_options opt = { 
    .max = 4000,
    .k = 3,
    .fn = &FN,
    .hmax = &hmax,
    .w_schedule = soo_schedule,
    .init_w = 1,
    .epsilon = 1e-4,
    .fn_optimum = FN_MAX,
  };
  return opt;
} /* test_soo() */

/***********************************************************
* test_logo
*
* Run the optimization using LOGO-like settings.
***********************************************************/
struct clogo_options test_logo()
{
  struct clogo_options opt = { 
    .max = 4000,
    .k = 3,
    .fn = &FN,
    .hmax = &hmax,
    .w_schedule = logo_schedule,
    .init_w = 3,
    .epsilon = 1e-4,
    .fn_optimum = FN_MAX,
  };
  return opt;
} /* test_logo() */

/***********************************************************
* main
***********************************************************/
int main() 
{
  struct clogo_options opt = test_soo();
  struct clogo_result result = clogo_optimize(&opt);
  display_result(&result);
  return 0;
} /* main() */
