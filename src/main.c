#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "clogo/clogo.h"

#define ROSENBROCK_MAX (0.0)
double rosenbrock_2(
  double *i
) 
{
double x = i[0], y = i[1];
double min = -5.0;
double max = 10.0;
x = min + x*(max-min);
y = min + y*(max-min);
return -(100.0*pow(y-x*x, 2.0)+pow(x*x-1.0, 2.0));

} /* rosenbrock_2() */


double sin_helper(
  double x
)
{
return (sin(13.0*x)*sin(27.0*x)+1.0)/2;

} /* sin_helper() */


#define SIN_MAX (0.9517936893872353)
double sin_2(
  double *i
)
{
double x = i[0], y = i[1];
return sin_helper(x)*sin_helper(y);

} /* sin_2() */


//TODO: Verify this is effectively identical to what's
//described in the paper
double hmax(
  int n
)
{
return sqrt((double)n);

} /* hmax() */


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


int soo_schedule(const struct cl_state *state)
{
return 1;

} /* soo_schedule() */


#define FN (&sin_2)
#define OPT (SIN_MAX)
void test_soo()
{
struct clogo_options opt = { 
  .max = 4000,
  .k = 3,
  .fn = FN,
  .hmax = &hmax,
  .w_schedule = soo_schedule,
  .init_w = 1,
  .epsilon = pow(10.0, -4.0),
  .fn_optimum = OPT,
};
clogo_test(&opt);

} /* test_soo() */

void test_logo()
{
struct clogo_options opt = { 
  .max = 4000,
  .k = 3,
  .fn = FN,
  .hmax = &hmax,
  .w_schedule = logo_schedule,
  .init_w = 3,
  .epsilon = pow(10.0, -4.0),
  .fn_optimum = OPT,
};
clogo_test(&opt);

} /* test_logo() */


int main() 
{
test_soo();
//test_logo();
return 0;
}
