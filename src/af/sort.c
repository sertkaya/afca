#include <assert.h>
#include <stdlib.h>
#include "af.h"

struct index_value {
	int index; // index of the argument
	double value; // value to use in sorting
};

struct index_value *index_value_pairs = NULL;

int cmp(const void *v1, const void *v2) {
	if ((((struct index_value*) v1)-> value) > (((struct index_value*) v2)-> value))
		return(1);
	else if ((((struct index_value*) v2)-> value) > (((struct index_value*) v1)-> value))
		return(-1);
	else
		return(0);
}

AF* sort_af(AF *af) {
	AF *s_af = create_argumentation_framework(af->size);

	index_value_pairs = calloc(af->size, sizeof(struct index_value));
	assert(index_value_pairs != NULL);

	int i,j;

	// number of attacks
	/*
	for (i = 0; i < c->size; ++i) {
		index_value_pairs[i].index = i;
		index_value_pairs[i].value = bitset_get_length(c->a[i]);
	}
	*/


	// number of attacked_by
	/*
	for (i = 0; i < c->size; ++i) {
		index_value_pairs[i].index = i;
		index_value_pairs[i].value = 0;
		for (j = 0; j < c->size; ++j)
			if (TEST_BIT(c->a[j], i))
				++index_value_pairs[i].value;
	}
	*/

	int attacks_count = 0, attacked_by_count = 0;
	for (i = 0; i < af->size; ++i) {
		index_value_pairs[i].index = i;
		attacks_count = count_bits(af->graph[i]);
		attacked_by_count = 0;
		for (j = 0; j < af->size; ++j)
			if (TEST_BIT(af->graph[j], i))
				++attacked_by_count;

		// if (attacks_count == 0)
		// 	attacks_count = 1;
		// index_value_pairs[i].value = ((double) attacks_count) / (0.5 * attacked_by_count);
		// index_value_pairs[i].value = ((double) attacked_by_count) / attacks_count;
		index_value_pairs[i].value = ((double) attacks_count) / attacked_by_count;
		// index_value_pairs[i].value = ((double) attacked_by_count) + 2 *  attacks_count;
		// index_value_pairs[i].value = ((double) attacks_count);
		// index_value_pairs[i].value = ((double) attacked_by_count) ;
	}

	// sort the index-value pairs according to value
	qsort(index_value_pairs, af->size, sizeof(index_value_pairs[0]), cmp);

	// fill in the new af sorted
	for (i = 0; i < af->size; ++i) {
		for (j = 0; j < af->size; ++j) {
			if (TEST_BIT(af->graph[index_value_pairs[i].index], index_value_pairs[j].index))
				SET_BIT(s_af->graph[i], j);
		}
	}

	// for (i = 0; i < c->size; ++i)
	//  	printf("%d %d %lf\n", i, index_value_pairs[i].index, index_value_pairs[i].value);

	return(s_af);
}

// map indices from sorted back to original
BitSet *map_indices_back(BitSet *s) {
  int i;
  BitSet* c = create_bitset(s->size);

  for (i = 0; i < s->size; ++i)
    if (TEST_BIT(s, i))
      SET_BIT(c, index_value_pairs[i].index);

  return(c);
}