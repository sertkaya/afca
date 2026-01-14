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

#ifndef AFCA_NODE_H
#define AFCA_NODE_H

#include "../af/af.h"

struct node {
    // ArrayList *set;
    bool* set;
    bool *processed;
    // Number of unattacked attackers of an argument
    SIZE_TYPE* unattacked_attackers_count;
    // Number of attackers of an argument that do not attack this node
    SIZE_TYPE* not_attacker_of_current_count;
    // Number of allowed attackers of an argument
    SIZE_TYPE* allowed_attackers_count;
    bool* victims;
    bool* attackers;
    // depth of this node
    int depth;
};

typedef struct node Node;

Node *create_node(SIZE_TYPE size);

Node *duplicate_node(Node *s, SIZE_TYPE size);

void delete_node(Node *s);

bool is_node_self_defending(Node* n, AF* af);

bool node_attacks_everything_outside(Node* n, AF* af);

// is arg in conflict with node->set
#define IS_IN_CONFLICT_WITH(arg,node) (node->victims[arg]||node->attackers[arg])

#endif //AFCA_NODE_H