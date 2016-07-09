#include "clogo/clogo.h"
#include "clogo/debug.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void clogo_test(
  struct clogo_options *opts
)
{
//Set up initial space
struct input_space space;
init_space(&space);

//Create topmost node
struct node *top = create_top_node();

//Put the topmost node on top
add_node_to_space(top, &space);


//Expand node
dbg_print_space(&space);
expand_node(top, &space, opts);
dbg_print_space(&space);

} /* clogo_test() */


void expand_node(
  struct node *n, 
  struct input_space *s, 
  struct clogo_options *opt
)
{
remove_node_from_space(n, s);
int split_dim = n->depth%DIM;

for (int i = 0; i < opt->k; i++) {
  struct node *child = create_child_node(n, opt->k, split_dim, i);
  add_node_to_space(child, s);
}

} /* expand_node() */


void init_space(
  struct input_space *s
)
{
s->capacity = 1;
s->depth_lists = malloc(sizeof(*s->depth_lists)*s->capacity);
for (int i = 0; i < s->capacity; i++) {
  init_node_list(&s->depth_lists[i]);
}

} /* init_space() */


void init_node_list(
  struct node_list *l
)
{
l->first = NULL;

} /* init_node_list() */


struct node * create_top_node(
  void
)
{
struct node *n = malloc(sizeof(*n));

for (int i = 0; i < DIM; i++) {
  n->edges[i] = 0.0;
  n->sizes[i] = 1.0;
}
n->depth = 0;
n->next = NULL;

return n;

} /* create_top_node() */


struct node * create_child_node(
  struct node *parent, 
  int splits,
  int split_dim,
  int idx
)
{
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
struct node_list *list = &s->depth_lists[n->depth];
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
struct node_list *l = &s->depth_lists[n->depth];
remove_node_from_list(n, l);

} /* remove_node_from_space() */


void grow_space(
  struct input_space *s
)
{
int new_capacity = s->capacity*2;
struct node_list *new_lists = malloc(sizeof(*new_lists)*new_capacity);
for (int i = 0; i < s->capacity; i++) {
  new_lists[i] = s->depth_lists[i];
}
for (int i = s->capacity; i < new_capacity; i++) {
  init_node_list(&new_lists[i]);
}
free(s->depth_lists);
s->depth_lists = new_lists;
s->capacity = new_capacity;

} /* grow_space() */
