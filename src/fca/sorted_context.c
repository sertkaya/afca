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

#include <assert.h>
#include "context.h"

struct index_value {
	int index; // the index of the object
	int value; // number of crosses for this object
};

int cmp(const void *v1, const void *v2) {
	if ((((struct index_value*) v1)-> value) > (((struct index_value*) v2)-> value))
		return(-1);
	else if ((((struct index_value*) v2)-> value) > (((struct index_value*) v1)-> value))
		return(1);
	else
		return(0);
}

Context* sort_context(Context *c) {
	Context *sc = create_context();
	init_context(sc, c->size);

	// find the number of crosses for each object, store in a struct
	struct index_value *index_value_pairs = calloc(c->size, sizeof(struct index_value));
	assert(index_value_pairs != NULL);

	int i;
	for (i = 0; i < c->size; ++i) {
		index_value_pairs[i].index = i;
		index_value_pairs[i].value = bitset_get_length(c->a[i]);
	}

	// sort the index-value pairs according to value
	qsort(index_value_pairs, c->size, sizeof(index_value_pairs[0]), cmp);


	// fill in the new context sorted
	int j;
	for (i = 0; i < c->size; ++i) {
		for (j = 0; j < c->size; ++j) {
			if (TEST_BIT(c->a[index_value_pairs[i].index], index_value_pairs[j].index))
				SET_BIT(sc->a[i], j);
		}
	}

	return(sc);
}
