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

#include <stdlib.h> //NULL, malloc, realloc
#include <string.h> //memcpy
#include <stdio.h> //printf
#include <math.h> //pow, log2, ceil
#include <stdbool.h> //for bools
#include <assert.h> //for assert
#include "my_allocator.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/


/* HeaderNode
   ----------
 * This is what our free list points to.  It is used to implement
 * the buddy system, and the splitting and coalescing parts.  It also
 * keeps track of our available free memory.
 *
 * The HeaderNodes are stored in our initially allocated memory, named Memory.
 * They are used to keep track of free space, so we will place them in not used
 * memory.
 * 
 * See Free List for more usage details.
 */
typedef struct HeaderNode HeaderNode;
struct HeaderNode{
  Addr nextNode;
  bool isRight;
  int inheritance;
};

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

static unsigned int FLBaseSize;
static unsigned int FLMaxSize;

/* Free list
   ---------
 * This free list keeps track of available memory spaces.  It a an array of HeaderNode pointers.
 * The index of the free list represents the size, with the space being 2^(index+base).
 * The base is derived from the user's basic block size, and the size of the free list is related
 * to the _length input.  Both the basic block size and overall length are rounded up to the next base 2
 * exponential for easy indexing.

 * Note the size is set with FLMaxSize, and the base is FLBaseSize.
 * Specifically the base size is the shift required to get the basic block size, 
 * so if the bbs is 5, we would raise to 8, and set the shift to 3, since 2^3 = 8.
 * This reduces the size of our free list.
 */
static HeaderNode* (*FreeList)[]; //an array of Header node pointers

/* Memory
   --------------------
 * We'll give the user the allocated size.  Note that this can be _bad_ if
 * the user's basic block size is less than the size of our HeaderNodes,
 * since we use the memory to store the free HeaderNodes.
 */
static Addr Memory;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */


/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS FOR INIT_ALLOCATOR() */
/*--------------------------------------------------------------------------*/

/* These functions are used in init_allocator() to find some initial values, constants
 * from user inputs _bbs, _length.
 */

/* This function takes a number, and log_2()'s it.  It returns the number rounded up.
 * Used to find the relative index in the free list.
 */
unsigned int getIndex2(const unsigned int input)
{
    assert(input >= 0);
    return (unsigned int)ceil(log2(input));
}


/* This function rounds the input to next highest power of 2.
 * Used for memory allocation, basic block size setting, calls to my_malloc.
 */
unsigned int roundUpPower2(const unsigned int input)
{
    return (unsigned int)pow(2, double(getIndex2(input)));
}

/* Note the currentIndex is the the shifted value
 * We need to guarantee that the currentFreeNode can fit the sizes... let's limit the bbs.
 *  
 */
void splitFreeList(HeaderNode* currentFreeNode, const unsigned int currentIndex) 
{
    assert(currentIndex >= 1);
    assert(currentFreeNode != NULL);

    //creating a new header node
    unsigned int halfLength = (pow(2, (currentIndex+FLBaseSize))) / 2;
    HeaderNode* newFreeNode = currentFreeNode + halfLength;
    memcpy(((void*)newFreeNode), (void*)currentFreeNode, sizeof(currentFreeNode));

    //linking our header nodes to the free list
    HeaderNode* freeListNode = (*FreeList)[currentIndex-1];
    while(freeListNode != NULL) {
        freeListNode = freeListNode->nextNode;
    }
    if (freeListNode == NULL)
        (*FreeList)[currentIndex-1] = currentFreeNode;
        currentFreeNode->nextNode = newFreeNode;
    }
    else {
        freeListNode->nextNode = currentFreeNode;
        (currentFreeNode->nextNode) = newFreeNode;
    }
    //buddy system setup
}

/* This function goes through the free list, returning a BinaryNode* 
 * to the location.  If there exists no free list available, then it goes up
 * until it finds it, breaks it down into the required size via split().
 * Returns NULL if the free list is has no available space.
 */
HeaderNode* getFreeNode(const unsigned int index)
{
    int adjustedIndex = index - FLBaseSize;
    HeaderNode* indexNode = (*FreeList)[adjustedIndex];
    if(indexNode == NULL) {
        for (int largerIndex = adjustedIndex; largerIndex <= FLMaxSize; largerIndex++) {
            indexNode = (*FreeList)[largerIndex];
            if (indexNode != NULL) {
                unsigned int currentIndex = largerIndex;
                while (currentIndex != adjustedIndex) {
                    splitFreeList(indexNode, currentIndex);
                    currentIndex -= 1;
                    indexNode = (*FreeList)[currentIndex];
                }
                return indexNode;
            }
        } //went through free list, can't find any bigger blocks
        return NULL;
    }
    else return indexNode;
}


/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS FOR INIT_ALLOCATOR() */
/*--------------------------------------------------------------------------*/

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

    assert(_basic_block_size >= 0 && _length >= 0);
    assert(_basic_block_size <= _length);

    //setting constants
    FLBaseSize = getIndex2(_basic_block_size);
    FLMaxSize = getIndex2(_length) - FLBaseSize;

    //setting the FreeList, memory
    FreeList = (HeaderNode* (*)[])(malloc(FLMaxSize * sizeof(HeaderNode*)));
    Memory = malloc(roundUpPower2(_length));

    //creating the initial header
    (*FreeList)[FLMaxSize] = (HeaderNode*)Memory; 
    (*FreeList)[FLMaxSize] -> nextNode = NULL;
    (*FreeList)[FLMaxSize] -> isRight = false;
    (*FreeList)[FLMaxSize] -> inheritance = -1;
   
    return roundUpPower2(_length);
}

int release_allocator() {
    // for (int i = 0; i < FLMaxSize; i++) {
    //     if (FreeList[i] != NULL)
    //         free FreeList[i];
    // }
    free(FreeList);
    free(Memory);
    return 0;
} 

extern Addr my_malloc(unsigned int _length) {
    /* This preliminary implementation simply hands the call over the 
       the C standard library! 
       Of course this needs to be replaced by your implementation.
    */
    // assert (_length <= roundUpPower2(FLMaxSize+FLBaseSize));
    // unsigned int index = getIndex2(_length);
    // HeaderNode* freeNode = getFreeNode(index);

    printf("HeaderNode size == %d\n", (int)sizeof(HeaderNode));


    return malloc((size_t)_length);
}

extern int my_free(Addr _a) {
    /* Same here! */
    free(_a);
    return 0;
}

