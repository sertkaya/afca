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

#include "../utils/stack.h"
#include "../af/af.h"
#include "node.h"

int closure_count = 0;

Node *pseudo_complete(Stack *update, Node *node, AF *af, AF* af_t) {
	SIZE_TYPE a = -1;
	++closure_count;
	while ((a = pop_int(update)) != -1) {

		if (IS_IN_CONFLICT_WITH(a, node)) {
			delete_node(node);
			return(NULL);
		}

		node->processed[a] = true;
		// list_add(a, node->set);
		node->set[a] = true;

		// semi-completion
		for (SIZE_TYPE i = 0; i < af->list_sizes[a]; ++i) {
			SIZE_TYPE victim_a = af->lists[a][i];
			if (!node->victims[victim_a]) {
				node->victims[victim_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[victim_a]; ++j) {
					SIZE_TYPE victim_victim_a = af->lists[victim_a][j];
					--(node->unattacked_attackers_count[victim_victim_a]);
					if (!node->attackers[victim_a]  && !node->processed[victim_a]) {
						--(node->allowed_attackers_count[victim_victim_a]);

						// Optimization for single attackers begins
						if (node->attackers[victim_victim_a] && !node->victims[victim_victim_a] && node->allowed_attackers_count[victim_victim_a] == 1) {
							// victim_victim_a is an unattacked attacker and has a single allowed attacker.
							// Push it immediately to the stack. It will anyway have to be
							// added later to make the set self-defending.
							for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_victim_a]; ++k) {
								if (!IS_IN_CONFLICT_WITH(af_t->lists[victim_victim_a][k], node) && !node->processed[af_t->lists[victim_victim_a][k]]) {
									push(update, new_stack_element_int(af_t->lists[victim_victim_a][k]));
									node->processed[af_t->lists[victim_victim_a][k]] = true;
								}
							}
						}
						// Optimization for single attackers ends

					}
					if (!node->processed[victim_victim_a] && node->unattacked_attackers_count[victim_victim_a] == 0)  {
						// victim_victim_a is defended, push it to the stack
						push(update, new_stack_element_int(victim_victim_a));
						node->processed[victim_victim_a] = true;
					}
				}
			}
		}

		// quasi-completion
		for (SIZE_TYPE i = 0; i < af_t->list_sizes[a]; ++i) {
			SIZE_TYPE attacker_a = af_t->lists[a][i];
			if (!node->attackers[attacker_a]) {
				node->attackers[attacker_a] = true;
				for (SIZE_TYPE j = 0; j < af->list_sizes[attacker_a]; ++j) {
					SIZE_TYPE victim_attacker_a = af->lists[attacker_a][j];
					--(node->not_attacker_of_current_count[victim_attacker_a]);
					if (!node->victims[attacker_a] && !node->processed[attacker_a]) {
						--(node->allowed_attackers_count[victim_attacker_a]);

						// Optimization for single attackers begins
						if (node->attackers[victim_attacker_a] && !node->victims[victim_attacker_a] && node->allowed_attackers_count[victim_attacker_a] == 1) {
							// victim_attacker_a is an unattacked attacker and has a single allowed attacker.
							// Push it immediately to the stack. It will anyway have to be
							// added later to make the set self-defending.
							for (SIZE_TYPE k = 0; k < af_t->list_sizes[victim_attacker_a]; ++k) {
								if (!IS_IN_CONFLICT_WITH(af_t->lists[victim_attacker_a][k], node) && !node->processed[af_t->lists[victim_attacker_a][k]]) {
									push(update, new_stack_element_int(af_t->lists[victim_attacker_a][k]));
									node->processed[af_t->lists[victim_attacker_a][k]] = true;
								}
							}
						}
						// Optimization for single attackers ends

					}
					if (!node->processed[victim_attacker_a] && node->not_attacker_of_current_count[victim_attacker_a] == 0) {
						// attackers of victim_attacker_a are all attackers of node->set, push it to the stack
						push(update, new_stack_element_int(victim_attacker_a));
						node->processed[victim_attacker_a] = true;
					}
				}
			}
		}

	}
	return(node);
}