#include <stdio.h>
#include <math.h>

#include "clogo/clogo.h"

double fn(double *i) 
{
  double x = i[0], y = i[1];
  return sin(4.0*(x+0.5*y))-x*x-y*y;
}

int main() {
  struct clogo_options opt = { 
    .k = 3,
    .fn = &fn
  };
  clogo_test(&opt);
  return 0;
}
