/*********************************************************************
* INCLUDES
*********************************************************************/
#include "clogo/clogo_private.h"
#include "clogo/debug.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


/*********************************************************************
* FUNCTIONS
*********************************************************************/

/***********************************************************
* clogo_optimize
*
* Current main interface to the clogo optimizer. Executes a 
* complete optimization process.
***********************************************************/
struct clogo_result clogo_optimize(
  const struct clogo_options *opt
                           //options describing the desired
                           //optimization
)
{
  //Set up and initialize the state to use for the entirety 
  //of the optimization.
  struct clogo_state state = clogo_init(opt);

  //Then, as long as the termination conditions aren't met,
  //continue expanding promising nodes.
  while (!clogo_done(&state)) {
    clogo_step(&state);
  }

  //Save the result before cleaning up the state.
  struct clogo_result result = clogo_finish(&state);
  clogo_delete(&state);
  return result;
} /* clogo_optimize() */

/***********************************************************
* clogo_init
*
* Initialize and return a optimization state structure to an
* appropriate state to begin the process.
***********************************************************/
struct clogo_state clogo_init(
  const struct clogo_options *opt
)
{
  struct clogo_state state = {
    .opt = opt,
    .samples = 0,
    .last_best_value = -INFINITY,
    .w = opt->init_w,
    .valid = true
  };

  //Create empty input space and populate it with a topmost 
  //node
  init_space(&state.space);
  struct node *top = create_top_node(&state);
  add_node_to_space(top, &state.space);

  return state;
} /* clogo_init() */

/***********************************************************
* clogo_step
*
* Execute one iteration of node expansion. Note that one
* call to this function may result in multiple samplings of
* the objective function.
***********************************************************/
void clogo_step(
  struct clogo_state *state
)
{
  assert(state->valid);

  //Select and expand nodes
  select_nodes(state);

  //Recalculate w according to the provided schedule
  //function.
  state->w = (*state->opt->w_schedule)(state);

  //Updated the best value seen so far-- this is 
  //currently only needed to inform the next iteration
  //of the w schedule.
  struct node *best = space_best_node(&state->space);
  state->last_best_value = best->value;
  
#ifdef DEBUG
  //Display the current best node for debug purposes
  printf("  Best: ");
  dbg_print_node(best);
#endif
} /* clogo_step() */

/***********************************************************
* clogo_done
*
* Returns true if the termination conditions have been met.
***********************************************************/
bool clogo_done(
  struct clogo_state *state
)
{
  const struct clogo_options *opt = state->opt;
  //Return true if we've:
  //Run out of samples OR
  //Achieved the desired error
  return state->samples >= opt->max ||
         state_error(state) <= opt->epsilon;
} /* clogo_done() */

/***********************************************************
* clogo_finish
*
* Returns a result structure for the optimization state.
***********************************************************/
struct clogo_result clogo_finish(
  struct clogo_state *state//optimization state to examine
)
{
  return make_result(state);
} /* clogo_finish() */

/***********************************************************
* clogo_delete
*
* Fully cleans up the specified state structure.
* Note that this doesn't delete the options structure.
***********************************************************/
void clogo_delete(
  struct clogo_state *state//state to be deleted
)
{
  struct space *space = &state->space;

  //Destructively delete each depth list
  for (int h = 0; h < space->capacity; h++) {
    struct node_list *list = &space->depth[h];  
    struct node *node = list->first;
    while (node != NULL) {
      struct node *next = node->next;
      free(node);
      node = next;
    }
  }

  //Delete the depth list itself
  free(space->depth);
} /* clogo_delete */

/***********************************************************
* state_best_value
*
* Returns the best value of any node at the current state
* of the optimization.
***********************************************************/
double state_best_value(
  const struct clogo_state *state
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
* select_nodes
*
* Iterates through each depth of the partitioned input space
* and expands the appropriate nodes.
***********************************************************/
void select_nodes(
  struct clogo_state *state   //state of the optimization proc-
                           //ess
)
{
  //Convenience alias for the optimization options.
  const struct clogo_options *opt = state->opt;
  //Convenience alias for the input space.
  struct space *space = &state->space;
  //The best value of a node up until the current point.
  double prev_best = -INFINITY;
  //Maximum value of `k` for this iteration. Note that this
  //is calculated based on the provided hmax function (which
  //calculates the maximum depth to reach) and the current
  //'depth width' (`w`) of the search. This way the max 
  //depth is never violated.
  int kmax = (int)((*opt->hmax)(state->samples)/state->w);

#ifdef DEBUG
  //Debug output to display that variables are being 
  //calculated correctly.
  printf("Selecting (n=%d, w=%d, kmax=%d):\n", state->samples, state->w, kmax);
#endif

  //Loop through each set of `w` depths.
  for (int k = 0; k <= kmax; k++) {
    //Best node observed so far in this set of depths.
    struct node *best = NULL;
    //Minimum/maximum depth included in this set.
    int h_min = k*state->w;
    int h_max = (k+1)*state->w-1;
    //For each depth level in the set of depths being 
    //considered...
    for (int h = h_min; h <= h_max; h++) {
      //Find the best node at this depth.
      struct node *h_best = depth_best_node(space, h);

      //Update the best node pointer if the best node
      //at our current level is better than the best
      //node found in the set so far.
      if (h_best == NULL) continue;
      if (best == NULL || h_best->value > best->value) {
        best = h_best; 
      }
    }

    //If the best node in this depth set is better than
    //every node in the depth sets ABOVE this one, expand
    //it!
    if (best != NULL && best->value > prev_best) {
      //Update the best-observed-so-far value so that future
      //expansions are correct.
      prev_best = best->value;

#ifdef DEBUG
      //Just a debug display to show the size of the depth 
      //sets, the depth level of the nodes selected for 
      //expansion, and the node itself.
      if (h_min != h_max) {
        printf("  Depth %d-%d (%d): ", h_min, h_max, best->depth);
      } else {
        printf("  Depth %d: ", best->depth);
      }
      dbg_print_node(best);
#endif

      //Expand the node-- this also increases the sample
      //count.
      double child_best = expand_and_remove_node(best, state);

      //Check termination conditions-- if either is 
      //violated, stop the selection at this point so that
      //no further work is done.
      //Since the program state is always kept in a 'good'
      //state, there's no need for cleanup or final 
      //processing-- we can just stop whenever we want and
      //examine the results later.
      if (state->samples >= opt->max) return;
      if (opt->fn_optimum - child_best < opt->epsilon) return;
    }
  }
} /* select_nodes() */

/***********************************************************
* make_result
*
* Build a result structure corresponding with the given
* optimization state.
***********************************************************/
struct clogo_result make_result(
  const struct clogo_state *state
)
{
  struct clogo_result result;
  struct node *best = space_best_node(&state->space);
  calculate_center(best, result.point);
  result.value = best->value;
  result.samples = state->samples;
  return result;
} /* make_result() */

/***********************************************************
* state_error
*
* Returns the error of the given state based on the best
* function value provided in the options structure.
***********************************************************/
double state_error(
  const struct clogo_state *state
                           //system state
)
{
  //Convenience alias for the optimization options.
  const struct clogo_options *opt = state->opt;

  //If we don't know what the optimum value is, we can't
  //calculate the error-- so just return maximum error.
  if (opt->fn_optimum == INFINITY) return INFINITY;

  //Find the best node in the space currently...
  struct node *space_best = space_best_node(&state->space);

  //...and calculate its error as described on p172 of the
  //paper.
  //NOTE: This should probably be changed in the future. 
  //It's just here for fair comparisons.
  double error;
  if (opt->fn_optimum == 0.0) {
    error = opt->fn_optimum-space_best->value;
  } else {
    error = (opt->fn_optimum-space_best->value)/opt->fn_optimum;
  }
  return error;
} /* state_error() */

/***********************************************************
* sample_node
*
* Fills out the given node structure with its appropriate
* function value. This is the only place the function in
* the options structure should be evaluated.
***********************************************************/
void sample_node(
  struct node *n,          //node to modify
  struct clogo_state *state//current optimization state
)
{
  double center[DIM];
  calculate_center(n, center);
  n->value = (*state->opt->fn)(center);
  state->samples++;
} /* sample_node() */

/***********************************************************
* expand_and_remove_node
*
* Expands the referenced node, adding its children to the
* next depth level of the input space and removing it from
* its current level.
* Also deletes the node being expanded.
* Returns the value of the best child node created.
***********************************************************/
double expand_and_remove_node(
  struct node *n,          //node to expand
  struct clogo_state *state   //system state
)
{
  //Convenience aliases
  struct space *space = &state->space;
  const struct clogo_options *opt = state->opt;
  //Best child value seen so far
  double best = -INFINITY;

  //First, yank the node being expanded out of the input 
  //space.
  remove_node_from_space(n, space);

  //Choose the dimension to split along. 
  //This is supposed to be the dimension with largest size 
  //in the parent cell but since the cells are all uniformly 
  //sized this approach (cycling through the dimensions as 
  //depth decreases) accomplishes the same thing and is 
  //easier.
  int split_dim = n->depth%DIM;

  //Ensure that there's an odd number of splits so the 
  //middle node can inherint the parent's value without
  //needing to do an extra function call.
  assert(opt->k % 2 == 1);
  for (int i = 0; i < opt->k; i++) {
    struct node *child = create_child_node(n, state, split_dim, i);
    add_node_to_space(child, space);
    if (child->value > best) best = child->value;

    //Jump out if the termination conditions have been met--
    //this technically leaves a 'hole' in the input space
    //that would make it impossible to continue, but we know
    //we're about to finish anyway so it's fine.
    if (state->samples >= opt->max ||
        opt->fn_optimum - best < opt->epsilon) {
      state->valid = false;
      break;
    }
    //TODO: add `valid` flag to state.
    //TODO: Check that the other term. cond. isn't violated
  }

  //Finally, delete the expanded and removed node.
  free(n);

  return best;
} /* expand_and_remove_node() */

/***********************************************************
* create_child_node
*
* Create and return a single child node based descended from 
* a parent.
***********************************************************/
struct node * create_child_node(
  const struct node *parent, 
                           //parent of returned node
  struct clogo_state *state,  //current state to modify
  int split_dim,           //index of the dimension to split
                           //on
  int idx                  //index of the current node being
                           //created
)
{
  //Convenience options structure alias.
  const struct clogo_options *opt = state->opt;
  //`k` value -- Number of children per split.
  int splits = opt->k;
  //Calculate the width of the dimension to shrink along.
  double width = parent->sizes[split_dim] / splits;
  //...and allocate space for the new node.
  struct node *n = malloc(sizeof(*n));

  //Each edge will be identical to the parents' edges, other
  //than that along the split dimension.
  for (int i = 0; i < DIM; i++) {
    double edge = parent->edges[i];
    if (i == split_dim) edge += width * idx;
    n->edges[i] = edge;
  }

  //Each size will be identical to the parents', except that
  //along the split dimension.
  for (int i = 0; i < DIM; i++) {
    n->sizes[i] = (i == split_dim) ? width : parent->sizes[i];
  }

  //Child nodes are one depth deeper than their parent.
  n->depth = parent->depth + 1;
  n->next = NULL;

  //If this is the middle node, its center is identical to
  //the parent's center-- so just steal the parent's value!
  //Otherwise, sample.
  if (idx == opt->k / 2) {
    n->value = parent->value;
  } else {
    sample_node(n, state);
  }

  return n; //Return the fully-created child node.
} /* create_child_node() */

/***********************************************************
* create_top_node
*
* Create and return a topmost node to place in an input -
* space. This needs a special function because all other 
* nodes are created based on a parent node.
***********************************************************/
struct node * create_top_node(
  struct clogo_state *state//optimization state
)
{
  struct node *n = malloc(sizeof(*n));

  //Topmost node has all edges at 0 (minimum) all sizes of 
  //1 (maximum).
  for (int i = 0; i < DIM; i++) {
    n->edges[i] = 0.0;
    n->sizes[i] = 1.0;
  }

  n->depth = 0;
  n->next = NULL;

  //Now that we know where the node is, calculate its value.
  sample_node(n, state); 

  return n;
} /* create_top_node() */

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
  //Current node being examined-- start at the beginning of
  //the list.
  struct node *n = l->first;
  //Best node observed so far.
  struct node *best = n;
  
  //As long as we're not at the end of the list, update
  //the best node observed so far.
  while (n != NULL) {
    if (n->value > best->value) {
      best = n;
    }
    n = n->next;
  }

  return best; //...and return it!
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
  //Best node observed so far.
  struct node *best = NULL;

  //Find the best node at each depth in the space and use it
  //to update the best node we've seen so far.
  for (int h = 0; h < s->capacity; h++) {
    struct node *b = depth_best_node(s, h);  
    if (b == NULL) continue;
    if (best == NULL || b->value > best->value) best = b;
  }

  return best; //...and return it.
} /* space_best_node() */

/***********************************************************
* depth_best_node
*
* Returns:
*   * The best node in the given partitioned space
*     at the specified depth of the node hierarchy.
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
    //If the depth being requested is outside of the current
    //capacity of the space, just return NULL-- no nodes
    //exist there anyway.
    return NULL;
  }
} /* depth_best_node() */

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
  //NOTE: Maybe we should start with a higher initial capac-
  //ity, but it actually doesn't matter.
  s->capacity = 1;
  s->depth = malloc(sizeof(*s->depth)*s->capacity);
  for (int i = 0; i < s->capacity; i++) {
    init_node_list(&s->depth[i]);
  }
} /* init_space() */

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
  //Make sure our space is deep enough to hold the node.
  while (s->capacity <= n->depth) grow_space(s);
  //Find the list that represents all nodes at that depth
  //in the space...
  struct node_list *list = &s->depth[n->depth];
  //...and add the node to it!
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
  //Since each node is part of a singly-linked list, we
  //have to keep track of the previous and current nodes
  //so we can hook them up correctly once we remove one.
  struct node *prev = NULL;
  struct node *current = l->first;
  while (current != NULL) {
    if (current == n) {
     if (prev != NULL) {
        prev->next = n->next;
     } else {
        l->first = n->next;
     }
     //We removed the node, so jump out.
     return;
    }
    prev = current;
    current = current->next;
  }

  //We didn't find the node to remove! Panic!
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
  //Make sure the node we're removing can possibly exist
  //in the current partitioned input space.
  assert(n->depth < s->capacity);

  //Find the list at the correct depth...
  struct node_list *l = &s->depth[n->depth];
  //...and remove the requested node from it.
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
  //Let's double capacity with each size increase, sure.
  int new_capacity = s->capacity*2;
  struct node_list *new_lists = malloc(sizeof(*new_lists)*new_capacity);

  //Copy over node lists from the previous depth lists, and 
  //initialize anythat didn't used to exist to empty.
  for (int i = 0; i < s->capacity; i++) {
    new_lists[i] = s->depth[i];
  }
  for (int i = s->capacity; i < new_capacity; i++) {
    init_node_list(&new_lists[i]);
  }

  //Delete the old depth lists, and point the space towards
  //the new, bigger one.
  free(s->depth);
  s->depth = new_lists;
  s->capacity = new_capacity;
} /* grow_space() */

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
