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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "array_list.h"

ArrayList* list_create() {
	ArrayList* l = (ArrayList*) malloc(sizeof(ArrayList));
	assert(l != NULL);
 	l->size = 0;
	l->elements = NULL;

	return l;
}

int list_free(ArrayList* l) {
	free(l->elements);
	int freed_bytes = l->size * sizeof(ARG_TYPE);
	free(l);
	freed_bytes+= sizeof(ArrayList);

	return(freed_bytes);
}

int list_reset(ArrayList* l) {
	free(l->elements);
	int freed_bytes = l->size * sizeof(ARG_TYPE);
	l->size = 0;
	l->elements = NULL;

	return(freed_bytes);
}

ArrayList* list_duplicate(ArrayList* l) {
	ArrayList* copy = list_create();
	for (SIZE_TYPE i = 0; i < l->size; ++i)
		list_add(l->elements[i], copy);
	return(copy);
}

// copy elements of l1 into l2.
// removes existing elements of l1.
void list_copy(ArrayList* l1, ArrayList* l2) {
	list_reset(l2);
	l2->size = l1->size;
	l2->elements = calloc(l2->size, sizeof(ARG_TYPE));
	assert(l2->elements != NULL);
	for (SIZE_TYPE i = 0; i < l2->size; ++i)
		l2->elements[i] = l1->elements[i];
}

int cmp(const void* arg_1, const void* arg_2) {
	if ((*(ARG_TYPE*) arg_1) < *((ARG_TYPE*) arg_2))
		return(-1);
	if ((*(ARG_TYPE*) arg_1) > *((ARG_TYPE*) arg_2))
		return(1);
	return(0);
}

void print_list(FILE* fp, ArrayList* l, char* end) {
	qsort(l->elements, l->size, sizeof(ARG_TYPE), cmp);
	fprintf(fp, "w ");
	for (int i = 0; i < l->size; ++i)
		fprintf(fp,"%d ", l->elements[i] + 1);
	fprintf(fp, "%s", end);
}

extern inline bool list_add(ARG_TYPE e, ArrayList* l);


extern inline bool list_remove(ARG_TYPE e, ArrayList* l);

ListIterator* list_iterator_create(ArrayList* l) {
	ListIterator* it = (ListIterator*) malloc(sizeof(ListIterator));
	assert(it != NULL);
	it->list = l;
	it->current_index = 0;

	return it;
}

extern inline ARG_TYPE list_iterator_next(ListIterator* it);

int list_iterator_free(ListIterator* it) {
	free(it);

	return sizeof(ListIterator);
}