#include "clogo/debug.h"

#include <stdio.h>


void dbg_print_node(
  struct node *n
) 
{
double center[DIM];
calculate_center(n, center);

printf(
  "%f/%f\t%f/%f\t%f\n", 
  center[0], center[1],
  n->sizes[0], n->sizes[1],
  n->value
);

} /* dbg_print_node() */


void dbg_print_space(
  struct input_space *s
) 
{
for (int i = 0; i < s->capacity; i++) {
  struct node_list *l = &s->depth_lists[i];
  struct node *n = l->first;
  if (n == NULL) continue;
  printf("Depth %d:\n", i);
  do {
    printf("\t");
    dbg_print_node(n);
  } while ((n = n->next) != NULL);
}

} /* dbg_print_space() */
