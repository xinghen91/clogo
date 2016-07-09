#include <stdio.h>
#include <math.h>

#include "clogo/clogo.h"

double fn(double *i) 
{
  double x = i[0], y = i[1];
  double min = -5.0;
  double max = 10.0;
  x = min + x*(max-min);
  y = min + y*(max-min);
  return -(100.0*pow(y-x*x, 2.0)+pow(x*x-1.0, 2.0));
}

double hmax(int n)
{
  return sqrt((double)n);
}

int main() {
  struct clogo_options opt = { 
    .max = 8000,
    .k = 3,
    .w = 1,
    .fn = &fn,
    .hmax = &hmax,
    .epsilon = pow(10.0, -4.0),
    .optimum = 0.0,
  };
  clogo_test(&opt);
  return 0;
}
