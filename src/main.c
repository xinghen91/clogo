#include <stdio.h>
#include <math.h>

#include "clogo/clogo.h"

double fn(double *i) 
{
  double x = i[0], y = i[1];
  return sin(4.0*(x+0.5*y))-x*x-y*y;
}

double hmax(int n)
{
  return sqrt((double)n);
}

int main() {
  struct clogo_options opt = { 
    .max = 200,
    .k = 3,
    .fn = &fn,
    .hmax = &hmax,
  };
  clogo_test(&opt);
  return 0;
}
