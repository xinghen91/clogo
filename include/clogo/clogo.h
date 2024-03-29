#pragma once

/*********************************************************************
* INCLUDES
*********************************************************************/
#include <stdbool.h>

/*********************************************************************
* CONSTANTS
*********************************************************************/
#define DIM 2


/*********************************************************************
* TYPES
*********************************************************************/
//Forward-declared types
struct clogo_state;

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
  int (*w_schedule)(const struct clogo_state *);
                           //w schedule function
  int init_w;              //w value at iteration 0
  double epsilon;          //max error before stopping
                           //INFINITY=run until max
  double fn_optimum;       //fn's max value
};

/***********************************************************
* clogo_result
*
* Structure that represents the result of a clogo 
* optimization.
***********************************************************/
struct clogo_result {
  double point[DIM];       //point of max value found
  double value;            //max value found
  int samples;             //number of samples observed
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
* clogo_state
*
* Complete state of the clogo optimization process. Used
* as an input to the w_schedule function.
***********************************************************/
struct clogo_state {
  const struct clogo_options *opt;
                           //options that define the optimi-
                           //zation process
  struct space space;      //current partitioned input space
  int samples;             //number of samples observed
  double last_best_value;  //best value observed in the pre-
                           //vious iteration
  int w;                   //current w value
  bool valid;              //true if the state can be used
                           //for further optimization steps
};


/*********************************************************************
* FUNCTION PROTOTYPES
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
);

/***********************************************************
* clogo_init
*
* Initialize and return a optimization state structure to an
* appropriate state to begin the process.
***********************************************************/
struct clogo_state clogo_init(
  const struct clogo_options *opt
                           //options that define the optimi-
                           //zation
);

/***********************************************************
* clogo_step
*
* Execute one iteration of node expansion. Note that one
* call to this function may result in multiple samplings of
* the objective function.
***********************************************************/
void clogo_step(
  struct clogo_state *state//optimization state to use
);

/***********************************************************
* clogo_done
*
* Returns true if the termination conditions have been met.
***********************************************************/
bool clogo_done(
  struct clogo_state *state//optimization state to check
);

/***********************************************************
* clogo_finish
*
* Returns a result structure for the optimization state.
***********************************************************/
struct clogo_result clogo_finish(
  struct clogo_state *state//optimization state to examine
);

/***********************************************************
* clogo_delete
*
* Fully cleans up the specified state structure.
* Note that this doesn't delete the options structure.
***********************************************************/
void clogo_delete(
  struct clogo_state *state//state to be deleted
);

/***********************************************************
* state_best_value
*
* Returns the best value of any node at the current state
* of the optimization.
***********************************************************/
double state_best_value(
  const struct clogo_state *state
                           //system state
);
