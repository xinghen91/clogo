/*********************************************************************
* INCLUDES
*********************************************************************/
#include "clogo/clogo.h"
#include "clogo/debug.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>


/*********************************************************************
* FUNCTIONS
*********************************************************************/

/***********************************************************
* clogo_test
*
* Current main interface to the clogo optimizer. Executes a 
* complete optimization process.
***********************************************************/
void clogo_test(
  const struct clogo_options *opt
                           //options describing the desired
                           //optimization
)
{
  //Set up initial state
  struct cl_state state;
  init_state(&state, opt);

  while (state.samples < opt->max && 
         state_error(&state) >= opt->epsilon) {
    select_nodes(&state);
    printf("  Best: ");
    struct node *best = space_best_node(&state.space);
    dbg_print_node(best);
    //Recalculate w according to the provided schedule
    state.w = (*opt->w_schedule)(&state);
    //Updated the best value seen so far
    state.last_best_value = best->value;
  }
  printf("Final Best (n=%d): ", state.samples);
  dbg_print_node(space_best_node(&state.space));
  printf("Error: %e\n", state_error(&state));
} /* clogo_test() */

/***********************************************************
* select_nodes
*
* Iterates through each depth of the partitioned input space
* and expands the appropriate nodes.
***********************************************************/
void select_nodes(
  struct cl_state *state   //state of the optimization proc-
                           //ess
)
{
  const struct clogo_options *opt = state->opt;
  struct space *space = &state->space;
  double prev_best = -INFINITY;
  int kmax = (int)((*opt->hmax)(state->samples)/state->w);

  printf("Selecting (n=%d, w=%d, kmax=%d):\n", state->samples, state->w, kmax);
  for (int k = 0; k <= kmax; k++) {
    struct node *best = NULL;
    int h_min = k*state->w;
    int h_max = (k+1)*state->w-1;
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
      expand_and_remove_node(best, state);
      dbg_print_node(best);
      //Check termination conditions
      if (state->samples >= opt->max) return;
      if (opt->fn_optimum - best->value < opt->epsilon) return;
    }
  }
} /* select_nodes() */

/***********************************************************
* list_best_node
*
* Returns the node in the given list with the highest value.
***********************************************************/
struct node * list_best_node(
  const struct node_list *l
                           //node list to examine
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

/***********************************************************
* space_best_node
*
* Returns the best node in the given partitioned space.
***********************************************************/
struct node * space_best_node(
  const struct space *s    //space to examine
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

/***********************************************************
* depth_best_node
*
* Returns:
*   * The best node in the given partitioned space
*     at the specified depth of the node heirarchy.
*   * NULL if no nodes exist at that depth.
***********************************************************/
struct node * depth_best_node(
  const struct space *s,   //space to examine
  int h                    //depth to 
)
{
  if (h < s->capacity) {
    return list_best_node(&s->depth[h]);
  } else {
    return NULL;
  }
} /* depth_best_node() */

/***********************************************************
* state_best_value
*
* Returns the best value of any node at the current state
* of the optimization.
***********************************************************/
double state_best_value(
  const struct cl_state *state
                           //system state
)
{
  struct node *n = space_best_node(&state->space);
  if (n != NULL) {
    return n->value;
  } else {
    return -INFINITY;
  }
} /* space_best_value() */

/***********************************************************
* state_error
*
* Returns the error of the given state based on the best
* function value provided in the options structure.
***********************************************************/
double state_error(
  const struct cl_state *state
                           //system state
)
{
  const struct clogo_options *opt = state->opt;

  if (opt->fn_optimum == INFINITY) return INFINITY;
  struct node *space_best = space_best_node(&state->space);
  double error;
  if (opt->fn_optimum == 0.0) {
    error = opt->fn_optimum-space_best->value;
  } else {
    error = (opt->fn_optimum-space_best->value)/opt->fn_optimum;
  }
  return error;

} /* space_error() */

/***********************************************************
* expand_and_remove_node
*
* Expands the referenced node, adding its children to the
* next depth level of the input space and removing it from
* its current level.
* Also deletes the node being expanded.
***********************************************************/
void expand_and_remove_node(
  struct node *n,          //node to exppand
  struct cl_state *state   //system state
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
} /* expand_and_remove_node() */

/***********************************************************
* init_node_list
*
* Initialize a given node list to an empty state.
***********************************************************/
void init_node_list(
  struct node_list *l      //list to initialize
)
{
  l->first = NULL;
} /* init_node_list() */

/***********************************************************
* init_space
*
* Initialize the given input space structure to an empty 
* state.
***********************************************************/
void init_space(
  struct space *s          //space to initialize
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
  state->last_best_value = -INFINITY;
  state->w = opt->init_w;

  //Create input space and create topmost node
  init_space(&state->space);
  struct node *top = create_top_node(opt);
  add_node_to_space(top, &state->space);

} /* init_state() */

/***********************************************************
* create_top_node
*
* Create and return a topmost node to place in an input -
* space. This needs a special function because all other 
* nodes are created based on a parent node.
***********************************************************/
struct node * create_top_node(
  const struct clogo_options *opt
                           //options that define the optimi-
                           //zation
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

/***********************************************************
* create_child_node
*
* Create and return a single child node based decended from 
* a parent.
***********************************************************/
struct node * create_child_node(
  const struct node *parent, 
                           //parent of returned node
  struct cl_state *state,  //current state to modify
  int split_dim,           //index of the dimension to split
                           //on
  int idx                  //index of the current node being
                           //created
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

/***********************************************************
* add_node_to_list
*
* Adds the given node to the beginning of the given list.
***********************************************************/
void add_node_to_list(
  struct node *n,          //node to add
  struct node_list *l      //list to modify
)
{
  n->next = l->first;
  l->first = n;
} /* add_node_to_list() */

/***********************************************************
* add_node_to_space
*
* Adds the given node to the appropriate location in the 
* given input space.
***********************************************************/
void add_node_to_space(
  struct node *n,          //node to add
  struct space *s          //space to contain the node
)
{
  while (s->capacity <= n->depth) grow_space(s);
  struct node_list *list = &s->depth[n->depth];
  add_node_to_list(n, list);
} /* add_node_to_space() */

/***********************************************************
* remove_node_from_list
*
* Removes the given node from the given node list (without
* deleting it).
***********************************************************/
void remove_node_from_list(
  const struct node *n,    //node to remove
  struct node_list *l      //list to modify
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

/***********************************************************
* remove_node_from_space
*
* Removes the given node from the given input space (also
* without deleting it).
***********************************************************/
void remove_node_from_space(
  const struct node *n,    //node to remove
  struct space *s          //space to modify
)
{
  assert(n->depth < s->capacity);
  struct node_list *l = &s->depth[n->depth];
  remove_node_from_list(n, l);
} /* remove_node_from_space() */

/***********************************************************
* grow_space
*
* Expand the capacity of the given input space.
***********************************************************/
void grow_space(
  struct space *s          //space to expand
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

/***********************************************************
* sample_node
*
* Fills out the given node structure with its appropriate
* function value. This is the only place the function in
* the options structure should be evaluated.
***********************************************************/
void sample_node(
  struct node *n,          //node to modify
  const struct clogo_options *opt
                           //options that define the optimi-
                           //zation
)
{
  double center[DIM];
  calculate_center(n, center);
  n->value = (*opt->fn)(center);
} /* sample_node() */


/***********************************************************
* calculate_center
*
* Fill out the `center` array with DIM elements with the
* center point of the given input node.
***********************************************************/
void calculate_center(
  const struct node *n,    //node to consider
  double *center           //output center point array
)
{
  for (int i = 0; i < DIM; i++) {
    center[i] = n->edges[i] + n->sizes[i]/2.0;
  }
} /* calculate_center() */
