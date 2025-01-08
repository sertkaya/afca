/*
 * AFCA - argumentation framework using closed sets
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

#ifndef STACK_H_
#define STACK_H_

#include <stdlib.h>
#include <assert.h>

// typedef union stack_element StackElement;
typedef struct stack Stack;

/*
union stack_element {
	int n;
	void* p;
};
*/

struct stack {
	int size;
	unsigned int* elements;
	// StackElement** elements;
};

void init_stack(Stack* s);

inline void push(Stack* s, unsigned int e) {
	unsigned int* tmp = realloc(s->elements, (s->size + 1) * sizeof(unsigned int));
	assert(tmp != NULL);
	s->elements = tmp;
	s->elements[s->size] = e;
	++s->size;
}

inline unsigned int pop(Stack* s) {
	unsigned int e;
	unsigned int* tmp;

	if (s->size == 0)
		return(-1);

	--s->size;
	e = s->elements[s->size];
	tmp = realloc(s->elements, (s->size) * sizeof(unsigned int));
	assert(tmp != NULL || s->size == 0);
	s->elements = tmp;

	return e;
}

inline void free_stack(Stack* s) {
	free(s->elements);
}

#endif /* STACK_H_ */
