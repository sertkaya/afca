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

#include "af.h"
#include "../bitset/bitset.h"
#include <stdbool.h>

AF* create_argumentation_framework(int size) {
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
	int i, freed_bytes = 0;
	for (i = 0; i < af->size; ++i) {
		freed_bytes += free_bitset(af->graph[i]);
	}
	free(af->graph);
	freed_bytes += (af->size * sizeof(BitSet*));
	free(af);
	freed_bytes += sizeof(AF);
	return(freed_bytes);
}


int free_projected_argumentation_framework(PAF* paf) {
	free(paf->index_mapping);
	int freed_bytes = paf->af->size * sizeof(unsigned short);
	freed_bytes += free_argumentation_framework(paf->af);
	free(paf);
	return freed_bytes + sizeof(PAF);
}


void print_argumentation_framework(AF* af) {
	int i;

	for (i = 0; i < af->size; ++i) {
		print_bitset(af->graph[i], stdout);
		printf("\n");
	}
}

AF* complement_argumentation_framework(AF *af ){
	AF* c_af = create_argumentation_framework(af->size);

	c_af->size = af->size;

	int i;
	for (i = 0; i < af->size; ++i)
		complement_bitset(af->graph[i], c_af->graph[i]);

	return(c_af);
}

AF* transpose_argumentation_framework(AF *af) {
	AF* t_af = create_argumentation_framework(af->size);

	t_af->size = af->size;

	int i,j;
	for (i = 0; i < af->size; ++i)
		for (j = 0; j < af->size; ++j)
			if (TEST_BIT(af->graph[i], j))
				SET_BIT(t_af->graph[j], i);
	return(t_af);
}

PAF* project_argumentation_framework(AF *af, bool *mask) {
	PAF *paf = calloc(1, sizeof(PAF));
	assert(paf != NULL);

    unsigned short size = 0;
    for (unsigned short i = 0; i < af->size; ++i) {
        if (mask[i]) {
            ++size;
        }
    }

	if (size == 0) {
		return NULL;
	}

    paf->index_mapping = calloc(size, sizeof(unsigned short));
    unsigned short j = 0;
    for (unsigned short i = 0; i < af->size; ++i) {
        if (mask[i]) {
            paf->index_mapping[j++] = i;
        }
    }

    paf->af = create_argumentation_framework(size);
    for (unsigned short i = 0; i < size; ++i) {
        for (unsigned short j = 0; j < size; ++j) {
            if (TEST_BIT(af->graph[paf->index_mapping[i]], paf->index_mapping[j])) {
				SET_BIT(paf->af->graph[i], j);
            }
        }
    }

	return paf;
}


BitSet* project_back(BitSet* bs, PAF* paf, unsigned short base_size) {
	BitSet* res = create_bitset(base_size);
	for (unsigned short i = 0; i < paf->af->size; ++i) {
		if (TEST_BIT(bs, i)) {
			SET_BIT(res, paf->index_mapping[i]);
		}
	}
	return res;
}