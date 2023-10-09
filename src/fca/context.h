/*
 * AFCA - argumentation framework using closed sets
 *
 * Copyright (C) Baris Sertkaya (sertkaya@fb2.fra-uas.de)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FCA_CONTEXT_H_
#define FCA_CONTEXT_H_

#include "../bitset/bitset.h"

typedef struct context Context;

// For this special case (argumentation framework) attributes and objects are same and
// are just indices. So a formal context is an array of bitsets.
struct context {
	int size;
	BitSet** a;
};

// Create an empty context with the given size.
Context* create_context();

// Free the memory allocated for the context c.
void free_context(Context* c);

// Allocate space for the context.
void init_context(Context* c, int size);

// Add attribute at index i to object at index j in the context c.
#define ADD_ATTRIBUTE(c,i,j)		SET_BIT(c->a[j-1],i-1)

// Remove attribute at index i from object at index j in the context c.
#define REMOVE_ATTRIBUTE(c,i,j)		RESET_BIT(c->a[j-1], i-1)

void print_context(Context* c);

// Compute the double prime of the attribute set bs and put the result in r
void down_up_arrow(Context* c, BitSet* bs, BitSet* r);

void up_arrow(Context* c, BitSet* bs, BitSet* r);
void down_arrow(Context* c, BitSet* bs, BitSet* r);

// Negate the incidence relation (just flip the bits) and return pointer
// to the new context.
Context* negate_context(Context* c);

// Swap the rows and the columns.
// Under the assumption, same number of rows and columns!
// (which is the case in argumentation frameworks)
Context* transpose_context(Context *c);


#endif /* FCA_CONTEXT_H_ */
