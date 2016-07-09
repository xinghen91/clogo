#include <stdio.h>

#include "clogo/clogo.h"

int main() {
  struct clogo_options opt = { .k = 3 };
  clogo_test(&opt);
  return 0;
}
