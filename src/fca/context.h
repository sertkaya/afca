/*
 * context.h
 *
 *  Created on: 21.11.2022
 *      Author: bs
 */

#ifndef FCA_CONTEXT_H_
#define FCA_CONTEXT_H_

#include "../bitset/bitset.h"

typedef struct context Context;

// For this special case (argumentation framework) attributes and objects are same and
// are just indices. So a formal context is an array of bitsets.
struct context {
	int size;
	BitSet* a;
};

// Create an empty context with the given size.
Context* create_context();

// Allocate space for the context.
void init_context(Context* c, int size);

// Add attribute at index i to object at index j in the context c.
#define ADD_ATTRIBUTE(c,i,j)		SET_BIT(c->a[j-1],i-1)

// Remove attribute at index i from object at index j in the context c.
#define REMOVE_ATTRIBUTE(c,i,j)		RESET_BIT(c->a[j-1], i-1)

void print_context(Context* c);

#endif /* FCA_CONTEXT_H_ */
