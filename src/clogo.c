#include "clogo/clogo.h"
#include "clogo/debug.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

void clogo_test(
  struct clogo_options *opts
)
{
//Set up initial space
struct input_space space;
init_space(&space);

//Create topmost node
struct node *top = create_top_node(opts);

//Put the topmost node on top
add_node_to_space(top, &space);

int n = 0;
while (n < opts->max) {
  select_nodes(&space, opts, &n);
  printf("  Best: ");
  dbg_print_node(space_best_node(&space));
}
printf("Final Best (n=%d): ", n);
dbg_print_node(space_best_node(&space));

} /* clogo_test() */


void select_nodes(
  struct input_space *s, 
  struct clogo_options *opt,
  int *sample_cnt
)
{
double prev_best = -INFINITY;
int hmax = (int)(*opt->hmax)(*sample_cnt);

printf("Selecting (n=%d, hmax=%d):\n", *sample_cnt, hmax);
for (int h = 0; h <= hmax; h++) {
  if (h > s->capacity) continue;
  struct node *best = list_best_node(&s->depth[h]);
  if (best != NULL && best->value > prev_best) {
    prev_best = best->value;
    printf("  Depth %d: ", h);
    expand_node(best, s, opt, sample_cnt);
    dbg_print_node(best);
    //TODO: Calculate best, check termination condition
    if (*sample_cnt >= opt->max) return;
  }
}

} /* select_nodes() */


struct node * list_best_node(
  struct node_list *l
)
{
struct node *best = l->first;
struct node *n = l->first;

if (best == NULL) return NULL;

while (n != NULL) {
  if (n->value > best->value) {
    best = n;
  }
  n = n->next;
}

return best;

} /* list_best_node() */


struct node * space_best_node(
  struct input_space *s
)
{
struct node *best = NULL;
for (int h = 0; h < s->capacity; h++) {
  struct node *b = list_best_node(&s->depth[h]);  
  if (b == NULL) continue;
  if (best == NULL || b->value > best->value) best = b;
}

return best;

} /* space_best_node() */


void expand_node(
  struct node *n, 
  struct input_space *s, 
  struct clogo_options *opt,
  int *sample_cnt
)
{
remove_node_from_space(n, s);
int split_dim = n->depth%DIM;

assert(opt->k % 2 == 1);
for (int i = 0; i < opt->k; i++) {
  struct node *child = create_child_node(n, opt, split_dim, i, sample_cnt);
  add_node_to_space(child, s);
}

} /* expand_node() */


void init_space(
  struct input_space *s
)
{
s->capacity = 1;
s->depth = malloc(sizeof(*s->depth)*s->capacity);
for (int i = 0; i < s->capacity; i++) {
  init_node_list(&s->depth[i]);
}

} /* init_space() */


void init_node_list(
  struct node_list *l
)
{
l->first = NULL;

} /* init_node_list() */


struct node * create_top_node(
    struct clogo_options *opts
)
{
struct node *n = malloc(sizeof(*n));

for (int i = 0; i < DIM; i++) {
  n->edges[i] = 0.0;
  n->sizes[i] = 1.0;
}

n->depth = 0;
n->next = NULL;
sample_node(n, opts);

return n;

} /* create_top_node() */


struct node * create_child_node(
  struct node *parent, 
  struct clogo_options *opt,
  int split_dim,
  int idx,
  int *sample_cnt
)
{
int splits = opt->k;
struct node *n = malloc(sizeof(*n));
double width = parent->sizes[split_dim] / splits;

for (int i = 0; i < DIM; i++) {
  double edge = parent->edges[i];
  if (i == split_dim) edge += width * idx;
  n->edges[i] = edge;
}

for (int i = 0; i < DIM; i++) {
  n->sizes[i] = (i == split_dim) ? width : parent->sizes[i];
}

n->depth = parent->depth+1;
n->next = NULL;

if (idx == opt->k/2) {
  n->value = parent->value;
} else {
  sample_node(n, opt);
  (*sample_cnt)++;
}

return n;

} /* create_child_node() */


void add_node_to_list(
  struct node *n, 
  struct node_list *l
)
{
n->next = l->first;
l->first = n;

} /* add_node_to_list() */


void add_node_to_space(
  struct node *n, 
  struct input_space *s
)
{
while (s->capacity <= n->depth) grow_space(s);
struct node_list *list = &s->depth[n->depth];
add_node_to_list(n, list);

} /* add_node_to_space() */


void remove_node_from_list(
  struct node *n, 
  struct node_list *l
)
{
struct node *prev = NULL;
struct node *next = l->first;
while (next != NULL) {
  if (next == n) {
   if (prev != NULL) {
      prev->next = n->next;
   } else {
      l->first = n->next;
   }
   //We removed the node, so jump out.
   return;
  }
  prev = next;
  next = next->next;
}

//We didn't find the node to remove!
assert(false);

} /* remove_node_from_list() */


void remove_node_from_space(
  struct node *n, 
  struct input_space *s
)
{
assert(n->depth < s->capacity);
struct node_list *l = &s->depth[n->depth];
remove_node_from_list(n, l);

} /* remove_node_from_space() */


void grow_space(
  struct input_space *s
)
{
int new_capacity = s->capacity*2;
struct node_list *new_lists = malloc(sizeof(*new_lists)*new_capacity);
for (int i = 0; i < s->capacity; i++) {
  new_lists[i] = s->depth[i];
}
for (int i = s->capacity; i < new_capacity; i++) {
  init_node_list(&new_lists[i]);
}
free(s->depth);
s->depth = new_lists;
s->capacity = new_capacity;

} /* grow_space() */


void sample_node(
  struct node *n, 
  struct clogo_options *opt
)
{
double center[DIM];
calculate_center(n, center);
n->value = (*opt->fn)(center);

} /* sample_node() */


void calculate_center(
  struct node *n, 
  double *center
)
{
for (int i = 0; i < DIM; i++) {
  center[i] = n->edges[i] + n->sizes[i]/2.0;
}

} /* calculate_center() */
