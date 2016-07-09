#include <stdio.h>
#include <math.h>

#include "clogo/clogo.h"

double fn(double *i) 
{
  double x = i[0], y = i[1];
  x = -5.0 + x*(10.0+5.0);
  y = -5.0 + y*(10.0+5.0);
  //return sin(4.0*(x+0.5*y))-x*x-y*y;
  return -(100.0*pow(y-x*x, 2.0)+pow(x*x-1.0, 2.0));
}

double hmax(int n)
{
  return sqrt((double)n);
}

int main() {
  struct clogo_options opt = { 
    .max = 600,
    .k = 3,
    .w = 1,
    .fn = &fn,
    .hmax = &hmax,
  };
  clogo_test(&opt);
  return 0;
}
