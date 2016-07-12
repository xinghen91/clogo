#pragma once

/*********************************************************************
* CONSTANTS
*********************************************************************/
#define DIM 2


/*********************************************************************
* TYPES
*********************************************************************/
//Forward-declared types
struct cl_state;

/***********************************************************
* clogo_options
*
* Structure that's used to define how a single optimization
* should behave. Never modified by the optimization itself.
***********************************************************/
struct clogo_options {
  int max;                 //max number of function samples
  int k;                   //number of splits per cell
  double (*fn)(double *);  //function to evaluate
  double (*hmax)(int);     //depth limit function
  int (*w_schedule)(const struct cl_state *);
                           //w schedule function
  int init_w;              //w value at iteration 0
  double epsilon;          //max error before stopping
                           //INFINITY=run until max
  double fn_optimum;       //fn's max value
};

/***********************************************************
* node
*
* A sampled value in the space.
*
* A single intrusive linked list is used since each node
* only needs to be in at most one list of nodes at a time.
* It just makes implementation that much easier, and maybe
* is easier on the cache once things start getting crazy.
*
* NOTE: Ideally node creation would be managed by some
* allocation system that can keep nodes at the same depth
* close in memory to avoid cache misses when iterating
* through the depth lists.
***********************************************************/
struct node {
  double edges[DIM];       //edge of cell in each dimension
  double sizes[DIM];       //size of cell ...
  double value;            //sampled value at the center
  int depth;               //depth in hierarchy
  struct node *next;       //intrusive linked list pointer
};

/***********************************************************
* node_list
*
* A list of nodes. Since this is implemented via intrusive
* linked list, we just need to hold the first node in the
* list. This structure just exists for flexibility or future
* expansion.
***********************************************************/
struct node_list {
  struct node *first;      //first node in the list
};

/***********************************************************
* space
*
* Input space for the problem. Constructed of a bunch of
* nodes at varying depths that cover the whole space.
*
* `depth` points to a dynamic array of `capacity` node
* lists, each holding all the cells at that level.
***********************************************************/
struct space {
  struct node_list *depth; //array of depth node lists
  int capacity;            //number of elements in `depth`
};

/***********************************************************
* cl_state
*
* Complete state of the clogo optimization process. Used
* as an input to the w_schedule function.
***********************************************************/
struct cl_state {
  const struct clogo_options *opt;
                           //options that define the optimi-
                           //zation process
  struct space space;      //current partitioned input space
  int samples;             //number of samples observed
  double last_best_value;  //best value observed in the pre-
                           //vious iteration
  int w;                   //current w value
};


/*********************************************************************
* FUNCTION PROTOTYPES
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
);

/***********************************************************
* select_nodes
*
* Iterates through each depth of the partitioned input space
* and expands the appropriate nodes.
***********************************************************/
void select_nodes(
  struct cl_state *state   //state of the optimization proc-
                           //ess
);

/***********************************************************
* list_best_node
*
* Returns the node in the given list with the highest value.
***********************************************************/
struct node * list_best_node(
  const struct node_list *l
                           //node list to examine
);

/***********************************************************
* space_best_node
*
* Returns the best node in the given partitioned space.
***********************************************************/
struct node * space_best_node(
  const struct space *s    //space to examine
);

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
);

/***********************************************************
* state_best_value
*
* Returns the best value of any node at the current state
* of the optimization.
***********************************************************/
double state_best_value(
  const struct cl_state *state
                           //system state
);

/***********************************************************
* state_error
*
* Returns the error of the given state based on the best
* function value provided in the options structure.
***********************************************************/
double state_error(
  const struct cl_state *state
                           //system state
);

/***********************************************************
* expand_and_remove_node
*
* Expands the referenced node, adding its children to the
* next depth level of the input space and removing it from
* its current level.
* Also deletes the node being expanded.
***********************************************************/
void expand_and_remove_node(
  struct node *n,          //node to expand
  struct cl_state *state   //system state
);

/***********************************************************
* init_space
*
* Initialize the given input space structure to an empty 
* state.
***********************************************************/
void init_space(
  struct space *s          //space to initialize
);

/***********************************************************
* init_node_list
*
* Initialize a given node list to an empty state.
***********************************************************/
void init_node_list(
  struct node_list *l      //list to initialize
);

/***********************************************************
* init_state
*
* Initialize a given optimization state structure to an
* appropriate state to begin the process.
***********************************************************/
void init_state(
  struct cl_state *state,  //state to initialize
  const struct clogo_options *opt
                           //options that define the optimi-
                           //zation
);

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
);

/***********************************************************
* create_child_node
*
* Create and return a single child node based descended from 
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
);


/***********************************************************
* add_node_to_list
*
* Adds the given node to the beginning of the given list.
***********************************************************/
void add_node_to_list(
  struct node *n,          //node to add
  struct node_list *l      //list to modify
);

/***********************************************************
* add_node_to_space
*
* Adds the given node to the appropriate location in the 
* given input space.
***********************************************************/
void add_node_to_space(
  struct node *n,          //node to add
  struct space *s          //space to contain the node
);

/***********************************************************
* remove_node_from_list
*
* Removes the given node from the given node list (without
* deleting it).
***********************************************************/
void remove_node_from_list(
  const struct node *n,    //node to remove
  struct node_list *l      //list to modify
);

/***********************************************************
* remove_node_from_space
*
* Removes the given node from the given input space (also
* without deleting it).
***********************************************************/
void remove_node_from_space(
  const struct node *n,    //node to remove
  struct space *s          //space to modify
);

/***********************************************************
* grow_space
*
* Expand the capacity of the given input space.
***********************************************************/
void grow_space(
  struct space *s          //space to expand
);

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
);

/***********************************************************
* calculate_center
*
* Fill out the `center` array with DIM elements with the
* center point of the given input node.
***********************************************************/
void calculate_center(
  const struct node *n,    //node to consider
  double *center           //output center point array
);
