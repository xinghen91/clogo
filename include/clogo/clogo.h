#pragma once

#define DIM 2

struct clogo_options {
  int max;
  int k;
  int w;
  double (*fn)(double *);
  double (*hmax)(int);
  double epsilon;
  double optimum;
};

struct node {
  double edges[DIM], sizes[DIM];
  double value;
  int depth;
  //Intrusive linked list
  struct node *next;
};

struct node_list {
  struct node *first;
  //Other data to track...
};

struct space {
  struct node_list *depth;
  int capacity;
};

struct cl_state {
  const struct clogo_options *opt;
  struct space space;
  int samples;
};

void clogo_test(const struct clogo_options *opt);
void select_nodes(struct cl_state *state);
struct node * list_best_node(const struct node_list *l);
struct node * space_best_node(const struct space *s);
struct node * depth_best_node(const struct space *s, int h);
double state_error(const struct cl_state *state);
void expand_and_remove_node(struct node *n, 
                 struct cl_state *state);
void init_space(struct space *s);
void init_node_list(struct node_list *l);
void init_state(struct cl_state *state, const struct clogo_options *opt);
struct node * create_top_node(const struct clogo_options *opt);
struct node * create_child_node(const struct node *parent, 
                                struct cl_state *state,
                                int split_dim,
                                int idx);
void add_node_to_list(struct node *n, struct node_list *l);
void add_node_to_space(struct node *n, struct space *s);
void remove_node_from_list(const struct node *n, struct node_list *l);
void remove_node_from_space(const struct node *n, struct space *s);
void grow_space(struct space *s);
void sample_node(struct node *n, const struct clogo_options *opt);
void calculate_center(const struct node *n, double *center);
