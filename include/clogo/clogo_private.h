#pragma once

/*********************************************************************
* INCLUDES
*********************************************************************/
#include "clogo/clogo.h"

/*********************************************************************
* FUNCTION PROTOTYPES
*********************************************************************/

/***********************************************************
* make_result
*
* Build a result structure corresponding with the given
* optimization state.
***********************************************************/
struct clogo_result make_result(
  const struct cl_state *state
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
