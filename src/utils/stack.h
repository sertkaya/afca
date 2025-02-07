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


union stack_element {
	int n;
	void* p;
};
typedef union stack_element StackElement;

struct stack {
	int size;
	StackElement** elements;
};
typedef struct stack Stack;

void init_stack(Stack* s);

inline void push(Stack* s, StackElement* e) {
	StackElement** tmp = realloc(s->elements, (s->size + 1) * sizeof(StackElement*));
	assert(tmp != NULL);
	s->elements = tmp;

	// --> for testing: queue
	// ++s->size;
	// for (int i = s->size - 1; i > 0; --i) {
	// 	s->elements[i] = s->elements[i-1];
	// }
	// s->elements[0] = e;
	// <--

	// --> stack
	s->elements[s->size] = e;
	++s->size;
	// <--
}

inline StackElement* new_stack_element_int(int i) {
	StackElement* e = calloc(1, sizeof(StackElement));
	assert(e != NULL);
	e->n = i;
	return(e);
}

inline StackElement* new_stack_element_ptr(void* p) {
	StackElement* e = calloc(1, sizeof(StackElement));
	assert(e != NULL);
	e->p = p;
	return(e);
}

inline unsigned int pop_int(Stack* s) {
	StackElement* e;
	StackElement** tmp;

	if (s->size == 0)
		return(-1);

	--s->size;
	e = s->elements[s->size];
	tmp = realloc(s->elements, (s->size) * sizeof(StackElement*));
	assert(tmp != NULL || s->size == 0);
	s->elements = tmp;

	unsigned int e_n = e->n;
	free(e);

	return(e_n);
}

inline void* pop_ptr(Stack* s) {
	StackElement* e;
	StackElement** tmp;

	if (s->size == 0)
		return(NULL);

	--s->size;
	e = s->elements[s->size];
	void* e_p = e->p;
	free(e);

	tmp = realloc(s->elements, (s->size) * sizeof(StackElement*));
	assert(tmp != NULL || s->size == 0);
	s->elements = tmp;

	return(e_p);
}
/*
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
*/

inline void free_stack(Stack* s) {
	for (unsigned int i = 0; i < s->size; ++i)
		free(s->elements[i]);
	free(s->elements);
}

#endif /* STACK_H_ */
