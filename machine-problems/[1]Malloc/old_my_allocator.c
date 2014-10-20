/* 
    File: my_allocator.c

    Author: Yinwei (Charlie) Zhang
            Department of Computer Science
            Texas A&M University
    Date  : 15.09.2014

    Modified: 

    This file contains the implementation of the module "MY_ALLOCATOR".

*/


/*--------------------------------------------------------------------------*/
/* ABOUT */
/*--------------------------------------------------------------------------*/

/*  *Overview*
    *--------------------------------------------------------------------------*
    This file implements my_allocator.  

    What my_allocator does is that it initally mallocs (creates) some specified memory size.
    Then you can "malloc", or get memory from that initial block of memory, via the my_malloc(..) method.
    Of course you can free the specified memory via my_free().  To free the original block, use
    release_allocator(). 

    Basically, after my_allocator is initiated, it functions like our own version of malloc and free.

    *Details*
    *--------------------------------------------------------------------------*
    We simplicity of calculation, we internally define our sizes to be in powers of 2.  

    We have one big initial chunk of memory, defined by the user.  In order to know where each block of memory (split) is,
    we add a header object before each chunk of memory.  So we take user defined size, calculate the number of headers required,
    and allocate that size.  We also create a free list, which is an array of header pointers denote the size for each of the 
    blocks, with the index being log_2(size).  



    *Implementation*
    *--------------------------------------------------------------------------*

    For good management of our big block of memory from init_allocator(..), we use Knuth's buddy system method.
    



*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <stdlib.h> //NULL, malloc
#include <stdio.h> //printf
#include <math.h> //pow, log2, ceil
#include <stdbool.h> //for bools
#include "my_allocator.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

typedef struct Header_node Header_node;
struct Header_node{
  Addr next_node;
  bool is_right;
  int inheritance;
};

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

static unsigned int free_list_base_log_size; // set in set_buddy_system_binary_indexes
static unsigned int free_list_max_log_size; 
/* same as above, this is the total binary size MINUS the base binary size */

/* -- our super important free list that keeps track of the allocated memory --
  Initialized in init_allocator(...)
  Freed in release_allocator(...)
*/
static Header_node (*free_list)[]; //an array of Header node pointers

/* -- the memory itself -- */
static Addr memory;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */


/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

/* -- For setting up the initial allocator, init_allocator(...) -- */

static unsigned int find_minimum_memory_amount(unsigned int _basic_block_size, unsigned int _length) {
    double _bbs_base_2 = ceil(log2(_basic_block_size));
    double number_of_blocks_with_decimals = ((double) _length) / (_bbs_base_2);
    unsigned int number_of_blocks = (unsigned int) (int) ceil(number_of_blocks_with_decimals);

    unsigned int memory_size_for_headers = sizeof(Header_node) * number_of_blocks;
    return _length + memory_size_for_headers;
}

static unsigned int find_minimum_binary_memory_amount(unsigned int _bbs, unsigned int _length) {
    unsigned int minimum_memory_amount = find_minimum_memory_amount(_bbs, _length);
    unsigned int exponent_amount = (unsigned)(int) (ceil(log2(minimum_memory_amount)));
    return (unsigned int) (int) (pow(2, exponent_amount));
}

/* -- NOTE -- sets mini global variables buddy_system_base_log_size, buddy_system_max_log_size */
static void set_free_list_binary_indexes(unsigned int _basic_block_size, unsigned int _length) {
    free_list_base_log_size = (unsigned int)(ceil(log2(_basic_block_size)));
    free_list_max_log_size = (unsigned int)(log2(find_minimum_binary_memory_amount(_basic_block_size, _length) - free_list_base_log_size));
}


/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

/* Don't forget to implement "init_allocator" and "release_allocator"! */

/* This function initializes the memory allocator and makes a portion of 
   ’_length’ bytes available. The allocator uses a ’_basic_block_size’ as 
   its minimal unit of allocation. The function returns the amount of 
   memory made available to the allocator. If an error occurred, 
   it returns 0. 
*/     
unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length) {

    //sets the base and max indexes, where 2^(index) = the memory size of the block
    set_free_list_binary_indexes(_basic_block_size, _length); 
    unsigned int total_memory_size = find_minimum_binary_memory_amount(_basic_block_size, _length);
   
    //setting the free_list, memory
    free_list = ((Header_node*)malloc(free_list_max_log_size * sizeof(Header_node*)));
    memory = malloc(total_memory_size);

    //creating the initial header
    free_list[free_list_max_log_size] = (Header_node*)memory;
    free_list[free_list_max_log_size] -> {.next_node = NULL, .is_right = false, .inheritance = -1};
   
    return _length;
}

int release_allocator() {
    for (int i = 0; i < free_list_max_log_size; i++) {
        if (free_list[i] != NULL)
            free free_list[i];
    }
    free free_list;
    free memory;
    return 0;
} 

extern Addr my_malloc(unsigned int _length) {
    /* This preliminary implementation simply hands the call over the 
       the C standard library! 
       Of course this needs to be replaced by your implementation.
    */
    return malloc((size_t)_length);
}

extern int my_free(Addr _a) {
    /* Same here! */
    free(_a);
    return 0;
}

