//
// Created by bs on 12.01.26.
//

#ifndef AFCA_NODE_H
#define AFCA_NODE_H

#include "../af/af.h"

struct node {
    ArrayList *set;
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

// is arg in conflict with node->set
#define IS_IN_CONFLICT_WITH(arg,node) (node->victims[arg]||node->attackers[arg])

#endif //AFCA_NODE_H