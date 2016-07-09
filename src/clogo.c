#include "clogo/clogo.h"
#include "clogo/debug.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

void clogo_test(
  struct clogo_options *opt
)
{
//Set up initial state
struct cl_state state;
init_state(&state, opt);

while (state.samples < opt->max && 
       state_error(&state) >= opt->epsilon) {
  select_nodes(&state);
  printf("  Best: ");
  dbg_print_node(space_best_node(&state.space));
}
printf("Final Best (n=%d): ", state.samples);
dbg_print_node(space_best_node(&state.space));
printf("Error: %e\n", state_error(&state));

} /* clogo_test() */


void select_nodes(
  struct cl_state *state
)
{
const struct clogo_options *opt = state->opt;
struct space *space = &state->space;
double prev_best = -INFINITY;
int kmax = (int)((*opt->hmax)(state->samples)/opt->w);

printf("Selecting (n=%d, kmax=%d):\n", state->samples, kmax);
for (int k = 0; k <= kmax; k++) {
  struct node *best = NULL;
  int h_min = k*opt->w;
  int h_max = (k+1)*opt->w-1;
  for (int h = h_min; h <= h_max; h++) {
    struct node *h_best = depth_best_node(space, h);
    if (h_best == NULL) continue;
    if (best == NULL || h_best->value > best->value) {
      best = h_best; 
    }
  }
  if (best != NULL && best->value > prev_best) {
    prev_best = best->value;
    if (h_min != h_max) {
      printf("  Depth %d-%d (%d): ", h_min, h_max, best->depth);
    } else {
      printf("  Depth %d: ", best->depth);
    }
    expand_node(best, state);
    dbg_print_node(best);
    //Check termination conditions
    if (state->samples >= opt->max) return;
    if (opt->optimum - best->value < opt->epsilon) return;
  }
}

} /* select_nodes() */


struct node * list_best_node(
  const struct node_list *l
)
{
struct node *n = l->first;
struct node *best = n;

while (n != NULL) {
  if (n->value > best->value) {
    best = n;
  }
  n = n->next;
}

return best;

} /* list_best_node() */


struct node * space_best_node(
  const struct space *s
)
{
struct node *best = NULL;
for (int h = 0; h < s->capacity; h++) {
  struct node *b = depth_best_node(s, h);  
  if (b == NULL) continue;
  if (best == NULL || b->value > best->value) best = b;
}

return best;

} /* space_best_node() */


struct node * depth_best_node(
  const struct space *s, 
  int h
) 
{
if (h < s->capacity) {
  return list_best_node(&s->depth[h]);
} else {
  return NULL;
}

} /* depth_best_node() */


double state_error(
  const struct cl_state *state
)
{
const struct clogo_options *opt = state->opt;
struct node *space_best = space_best_node(&state->space);
double error;
if (opt->optimum == 0.0) {
  error = opt->optimum-space_best->value;
} else {
  error = opt->optimum - space_best->value/opt->optimum;
}
return error;

} /* space_error() */


void expand_node(
  struct node *n, 
  struct cl_state *state
)
{
struct space *space = &state->space;
const struct clogo_options *opt = state->opt;
remove_node_from_space(n, space);
int split_dim = n->depth%DIM;

assert(opt->k % 2 == 1);
for (int i = 0; i < opt->k && state->samples < opt->max; i++) {
  struct node *child = create_child_node(n, state, split_dim, i);
  add_node_to_space(child, space);
}

free(n);

} /* expand_node() */


void init_space(
  struct space *s
)
{
s->capacity = 1;
s->depth = malloc(sizeof(*s->depth)*s->capacity);
for (int i = 0; i < s->capacity; i++) {
  init_node_list(&s->depth[i]);
}

} /* init_space() */


void init_state(
  struct cl_state *state,
  const struct clogo_options *opt
)
{
//Assign options
state->opt = opt;

//Reset sample count
state->samples = 0;

//Create input space and create topmost node
init_space(&state->space);
struct node *top = create_top_node(opt);
add_node_to_space(top, &state->space);

} /* init_state() */


void init_node_list(
  struct node_list *l
)
{
l->first = NULL;

} /* init_node_list() */


struct node * create_top_node(
    const struct clogo_options *opt
)
{
struct node *n = malloc(sizeof(*n));

for (int i = 0; i < DIM; i++) {
  n->edges[i] = 0.0;
  n->sizes[i] = 1.0;
}

n->depth = 0;
n->next = NULL;
sample_node(n, opt);

return n;

} /* create_top_node() */


struct node * create_child_node(
  struct node *parent, 
  struct cl_state *state,
  int split_dim,
  int idx
)
{
const struct clogo_options *opt = state->opt;
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
  state->samples++;
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
  struct space *s
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
  struct space *s
)
{
assert(n->depth < s->capacity);
struct node_list *l = &s->depth[n->depth];
remove_node_from_list(n, l);

} /* remove_node_from_space() */


void grow_space(
  struct space *s
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
  const struct clogo_options *opt
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
