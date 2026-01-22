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
#include <string.h>
#include "../af/af.h"
#include  "node.h"

Node *create_node(SIZE_TYPE size) {
	Node *s = calloc(1, sizeof(Node));
	assert(s != NULL);

	// s->set = list_create();
	s->set = calloc(size, sizeof(bool));
	assert(s->set != NULL);

	s->processed = calloc(size, sizeof(bool));
	assert(s->processed != NULL);

	s->unattacked_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->unattacked_attackers_count != NULL);

	s->not_attacker_of_current_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->not_attacker_of_current_count != NULL);

	s->allowed_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(s->allowed_attackers_count != NULL);

	s->victims = calloc(size, sizeof(bool));
	assert(s->victims != NULL);

	s->attackers = calloc(size, sizeof(bool));
	assert(s->attackers != NULL);

	s->candidates = calloc(size, sizeof(bool));
	assert(s->candidates != NULL);

	s->depth = 0;

	return(s);
}

Node *duplicate_node(Node *s, SIZE_TYPE size) {
	Node *n = calloc(1, sizeof(Node));
	assert(n != NULL);

	// n->set = list_duplicate(s->set);
	n->set = calloc(size, sizeof(bool));
	assert(n->set != NULL);
	memcpy(n->set, s->set, size * sizeof(bool));

	n->processed = calloc(size, sizeof(bool));
	assert(n->processed != NULL);
	memcpy(n->processed, s->processed, size * sizeof(bool));

	n->unattacked_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(n->unattacked_attackers_count != NULL);
	memcpy(n->unattacked_attackers_count, s->unattacked_attackers_count, size * sizeof(SIZE_TYPE));

	n->not_attacker_of_current_count = calloc(size, sizeof(SIZE_TYPE));
	assert(n->not_attacker_of_current_count != NULL);
	memcpy(n->not_attacker_of_current_count, s->not_attacker_of_current_count, size * sizeof(SIZE_TYPE));

	n->allowed_attackers_count = calloc(size, sizeof(SIZE_TYPE));
	assert(n->allowed_attackers_count != NULL);
	memcpy(n->allowed_attackers_count, s->allowed_attackers_count, size * sizeof(SIZE_TYPE));

	n->victims = calloc(size, sizeof(bool));
	assert(n->victims != NULL);
	memcpy(n->victims, s->victims, size * sizeof(bool));

	n->attackers = calloc(size, sizeof(bool));
	assert(n->attackers != NULL);
	memcpy(n->attackers, s->attackers, size * sizeof(bool));

	n->candidates = calloc(size, sizeof(bool));
	assert(n->candidates != NULL);
	memcpy(n->candidates, s->candidates, size * sizeof(bool));

	n->depth = s->depth;

	return(n);
}

void delete_node(Node *s) {
	// list_free(s->set);
	// s->set = NULL;
	free(s->set);
	s->set = NULL;
	free(s->processed);
	s->processed = NULL;
	free(s->victims);
	s->victims = NULL;
	free(s->attackers);
	s->attackers = NULL;
	free(s->unattacked_attackers_count);
	s->unattacked_attackers_count = NULL;
	free(s->not_attacker_of_current_count);
	s->not_attacker_of_current_count = NULL;
	free(s->allowed_attackers_count);
	s->allowed_attackers_count = NULL;
	free(s->candidates);
	s->candidates = NULL;
	free(s);
}

inline bool is_node_self_defending(Node* n, AF* af) {
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (n->attackers[i] && !n->victims[i]) {
			// argument i is an attacker but not a victim, node
			// is not self-defending
			return(false);
		}
	}
	return(true);
}

inline bool node_attacks_everything_outside(Node* n, AF* af) {
	for (SIZE_TYPE i = 0; i < af->size; ++i) {
		if (!n->set[i] && !n->victims[i]) {
			// argument i not in the set and it is not a victim.
			return(false);
		}
	}
	return(true);
}
