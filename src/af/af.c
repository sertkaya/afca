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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>

#include "af.h"
#include "../bitset/bitset.h"
#include "../utils/timer.h"

AF* create_argumentation_framework(SIZE_TYPE size) {
	AF *af = calloc(1, sizeof(AF));
	assert(af != NULL);

	af->size = size;

	af->graph = (BitSet**) calloc(size, sizeof(BitSet*));
	assert(af->graph != NULL);

	int i;
	for (i = 0; i < size; ++i) {
		af->graph[i] = create_bitset(size);
	}


	return(af);
}

int free_argumentation_framework(AF* af) {
	int freed_bytes = 0;
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		freed_bytes += free_bitset(af->graph[i]);
	}
	free(af->graph);
	freed_bytes += (af->size * sizeof(BitSet*));
	free(af);
	freed_bytes += sizeof(AF);
	return(freed_bytes);
}


void free_paf(PAF* paf, bool free_af) {
	free(paf->base_mapping);
	free(paf->parent_mapping);
	if (free_af) {
		free_argumentation_framework(paf->af);
	}
	free(paf);
}

void free_projected_argumentation_framework(PAF* paf) {
	free_paf(paf, true);
}


void print_argumentation_framework(AF* af) {
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		print_bitset(af->graph[i], stdout);
		printf("\n");
	}
}

AF* complement_argumentation_framework(AF *af ){
	AF* c_af = create_argumentation_framework(af->size);

	c_af->size = af->size;

	for (SIZE_TYPE i = 0; i < af->size; ++i)
		complement_bitset(af->graph[i], c_af->graph[i]);

	return(c_af);
}

AF* transpose_argumentation_framework(AF *af) {
	struct timeval start_time, stop_time;
	START_TIMER(start_time);
	AF* t_af = create_argumentation_framework(af->size);

	t_af->size = af->size;

	for (SIZE_TYPE i = 0; i < af->size; ++i)
		for (SIZE_TYPE j = 0; j < af->size; ++j)
			if (TEST_BIT(af->graph[i], j))
				SET_BIT(t_af->graph[j], i);
	STOP_TIMER(stop_time);
	printf("Transposing AF time: %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	return(t_af);
}

AF* create_conflict_framework(AF* af) {
	// make it undirected and take loops into account
	AF* conflicts = create_argumentation_framework(af->size);

	conflicts->size = af->size;

	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		bool iconflict = TEST_BIT(af->graph[i], i);
		for (SIZE_TYPE j = i; j < af->size; ++j) {
			if (iconflict || TEST_BIT(af->graph[i], j) || TEST_BIT(af->graph[j], i) || TEST_BIT(af->graph[j], j)) {
				SET_BIT(conflicts->graph[i], j);
				SET_BIT(conflicts->graph[j], i);
			}
		}
	}

	return conflicts;
}

PAF* project_argumentation_framework_with_loops(AF* af, BitSet* mask, BitSet* loop_mask) {
	PAF* paf = calloc(1, sizeof(PAF));
	assert(paf != NULL);

    SIZE_TYPE size = count_bits(mask);
	assert(size > 0);

    paf->base_mapping = calloc(size, sizeof(SIZE_TYPE));
    paf->parent_mapping = calloc(size, sizeof(SIZE_TYPE));
    SIZE_TYPE j = 0;
    for (SIZE_TYPE i = 0; i < af->size; ++i) {
        if (TEST_BIT(mask, i)) {
			paf->parent_mapping[j] = i;
            paf->base_mapping[j++] = i;
        }
    }

    paf->af = create_argumentation_framework(size);
    for (SIZE_TYPE i = 0; i < size; ++i) {
		if (loop_mask && TEST_BIT(loop_mask, paf->parent_mapping[i])) {
			SET_BIT(paf->af->graph[i], i);
		}
        for (SIZE_TYPE j = 0; j < size; ++j) {
            if (CHECK_ARG_ATTACKS_ARG(af, paf->parent_mapping[i], paf->parent_mapping[j])) {
				SET_BIT(paf->af->graph[i], j);
            }
        }
    }

	paf->base_size = af->size;

	return paf;
}


PAF* project_argumentation_framework(AF *af, BitSet *mask) {
	return project_argumentation_framework_with_loops(af, mask, NULL);
}


PAF* project_paf_with_loops(PAF* paf, BitSet* mask, BitSet* loop_mask) {
	PAF* proj = calloc(1, sizeof(PAF));
	assert(proj != NULL);

    SIZE_TYPE size = count_bits(mask);
	assert(size > 0);

    proj->base_mapping = calloc(size, sizeof(SIZE_TYPE));
    proj->parent_mapping = calloc(size, sizeof(SIZE_TYPE));
    SIZE_TYPE j = 0;
    for (SIZE_TYPE i = 0; i < paf->af->size; ++i) {
        if (TEST_BIT(mask, i)) {
			proj->parent_mapping[j] = i;
            proj->base_mapping[j++] = paf->base_mapping[i];
        }
    }

    proj->af = create_argumentation_framework(size);
	bool loops = 0;
    for (SIZE_TYPE i = 0; i < size && loops < size; ++i) {
		if (loop_mask && TEST_BIT(loop_mask, proj->parent_mapping[i])) {
			SET_BIT(proj->af->graph[i], i);
			++loops;
		}
        for (SIZE_TYPE j = 0; j < size; ++j) {
            if (CHECK_ARG_ATTACKS_ARG(paf->af, proj->parent_mapping[i], proj->parent_mapping[j])) {
				SET_BIT(proj->af->graph[i], j);
            }
        }
    }

	if (loops == size) {
		free_projected_argumentation_framework(proj);
		proj = NULL;
	} else {
		proj->base_size = paf->base_size;
	}

	return proj;
}

PAF* af2paf(AF* af)
{
	PAF* paf = calloc(1, sizeof(PAF));
	paf->af = af;
	paf->base_mapping = calloc(af->size, sizeof(SIZE_TYPE));
	paf->parent_mapping = calloc(af->size, sizeof(SIZE_TYPE));
    for (SIZE_TYPE i = 0; i < af->size; ++i) {
        paf->base_mapping[i] = paf->parent_mapping[i] = i;
    }
	paf->base_size = af->size;
	return paf;
}


BitSet* project_back(BitSet* bs, PAF* paf) {
	BitSet* res = create_bitset(paf->base_size);
	for (SIZE_TYPE i = 0; i < paf->af->size; ++i) {
		if (TEST_BIT(bs, i)) {
			SET_BIT(res, paf->base_mapping[i]);
		}
	}
	return res;
}


void swap_arguments(AF* af, SIZE_TYPE i, SIZE_TYPE j) {
	BitSet* iset = af->graph[i];
	af->graph[i] = af->graph[j];
	af->graph[j] = iset;

	for (SIZE_TYPE k = 0; k < af->size; ++k) {
		bool ibit = TEST_BIT(af->graph[k], i);
		if (TEST_BIT(af->graph[k], j)) {
			SET_BIT(af->graph[k], i);
		} else {
			RESET_BIT(af->graph[k], i);
		}
		if (ibit) {
			SET_BIT(af->graph[k], j);
		} else {
			RESET_BIT(af->graph[k], j);
		}
	}
}

// Map indices of bitset s according to the mapping.
// Return the new bitset
BitSet *map_indices(BitSet *s, int *mapping) {
  int i;
  BitSet* c = create_bitset(s->size);

  for (i = 0; i < s->size; ++i)
    if (TEST_BIT(s, i))
      SET_BIT(c, mapping[i]);

  return(c);
}