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
 * A simple list implementation that supports adding and removing elements.
 */

#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>
#include <assert.h>

typedef struct array_list ArrayList;

struct array_list {
	int size;
	void** elements;
};

/**
 * Create list.
 * Returns a pointer to the created list.
 */
ArrayList* array_list_create();

/**
 * Appends element e to list l. Does not check for duplicates.
 * Returns 1.
 */
inline char array_list_add(void* e, ArrayList* l) {
	void** tmp = realloc(l->elements, (l->size + 1) * sizeof(void*));
	assert(tmp != NULL);
	l->elements = tmp;
	l->elements[l->size] = e;
	++l->size;

	return 1;
}

/**
 * Free the space allocated for this list.
 */
int array_list_free(ArrayList* l);

#endif
