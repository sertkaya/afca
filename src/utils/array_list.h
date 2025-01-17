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

/**
 * List of integers.
 */

#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "../af/datatypes.h"

typedef struct array_list ArrayList;
typedef struct list_iterator ListIterator;

struct array_list {
	SIZE_TYPE size;
	ARG_TYPE* elements;
};

struct list_iterator {
	ArrayList* list;
	SIZE_TYPE current_index;
};

/**
 * Create list.
 * Returns a pointer to the created list.
 */
ArrayList* list_create();

/**
 * Initialize a given list. Intended for not dynamically created
 * lists.
 */
// inline void list_init(List* l);
#define LIST_INIT(l)		do {((List*) l)->size=0; ((List*) l)->elements=NULL;} while(0)
/**
 * Appends element e to list l. Does not check for duplicates.
 * Returns 1.
 */
inline bool list_add(ARG_TYPE e, ArrayList* l) {
	ARG_TYPE* tmp = realloc(l->elements, (l->size + 1) * sizeof(ARG_TYPE));
	assert(tmp != NULL);
	l->elements = tmp;
	l->elements[l->size] = e;
	++l->size;

	return(true);
}

/**
 * Removes the first occurrence of the element e if it is present. The list stays unchanged
 * if e does not occur in l.
 * Returns 1 if e is removed, 0 otherwise.
 */
/**
 * TODO
 */
inline bool list_remove(ARG_TYPE e, ArrayList* l) {
	int i;

	for (i = 0; i < l->size; ++i) {
		if (e == l->elements[i])
			// the element is at index i
			break;
	}
	// if we reached the last element and it is not e,
	// then e does not exist in l.
	if ((i == l->size - 1) && (l->elements[i] != e))
		return(false);
	// now shift the elements, overwrite index i
	int j;
	for (j = i; j < l->size - 1; ++j) {
		l->elements[j] = l->elements[j + 1];
	}
	// shrink the allocated space
	ARG_TYPE* tmp = realloc(l->elements, (l->size - 1) * sizeof(ARG_TYPE));
	assert(l->size == 1 || tmp != NULL);
	l->elements = tmp;
	// decrement the element count
	--l->size;
	return(true);
}
/**
 * Free the space allocated for this list.
 */
int list_free(ArrayList* l);

/**
 * Free the space allocated for the elements of the given list.
 */
int list_reset(ArrayList* l);

/**
 * Creates an iterator for the given list.
 * Returns the created iterator.
 */
ListIterator* list_iterator_create(ArrayList* l);


/**
 * Returns the next element of the given iterator.
 * -1 if there is no next element.
 */
inline ARG_TYPE list_iterator_next(ListIterator* it) {
	ARG_TYPE next = (it->current_index == it->list->size) ? -1 : it->list->elements[it->current_index];
	++it->current_index;

	return(next);
}

void print_list(FILE* fp, ArrayList* l, char* end);

ArrayList* list_duplicate(ArrayList* l);

void list_copy(ArrayList* l1, ArrayList* l2);
#endif
