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

struct input_space {
  struct node_list *depth;
  int capacity;
};

void clogo_test(struct clogo_options *opts);
void select_nodes(struct input_space *i, 
                  struct clogo_options *opt,
                  int *n);
struct node * list_best_node(struct node_list *l);
struct node * space_best_node(struct input_space *s);
double space_error(struct input_space *s, struct clogo_options *opts);
void expand_node(struct node *n, 
                 struct input_space *s, 
                 struct clogo_options *opt,
                 int *sample_cnt);
void init_space(struct input_space *s);
void init_node_list(struct node_list *l);
struct node * create_top_node(struct clogo_options *opts);
struct node * create_child_node(struct node *parent, 
                                struct clogo_options *opt,
                                int split_dim,
                                int idx,
                                int *sample_cnt);
void add_node_to_list(struct node *n, struct node_list *l);
void add_node_to_space(struct node *n, struct input_space *s);
void remove_node_from_list(struct node *n, struct node_list *l);
void remove_node_from_space(struct node *n, struct input_space *s);
void grow_space(struct input_space *s);
void sample_node(struct node *n, struct clogo_options *opt);
void calculate_center(struct node *n, double *center);
