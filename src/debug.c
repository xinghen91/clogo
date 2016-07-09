#include "clogo/debug.h"

#include <stdio.h>


void dbg_print_node(
  struct node *n
) 
{
double center[DIM];
calculate_center(n, center);

printf(
  "%f/%f\t%e/%e\t%e\n", 
  center[0], center[1],
  n->sizes[0], n->sizes[1],
  n->value
);

} /* dbg_print_node() */

void dbg_print_node_list(
  struct node_list *l
)
{
struct node *n = l->first;
do {
  printf("\t");
  dbg_print_node(n);
} while ((n = n->next) != NULL);

} /* dbg_print_node_list() */


void dbg_print_space(
  struct input_space *s
) 
{
printf("=====\n");
for (int i = 0; i < s->capacity; i++) {
  struct node_list *l = &s->depth[i];
  if (s->depth[i].first == NULL) continue;
  printf("Depth %d:\n", i);
  dbg_print_node_list(l);
}

} /* dbg_print_space() */
