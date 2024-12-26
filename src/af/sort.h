//
// Created by bs on 13.09.24.
//

#ifndef SORT_H
#define SORT_H
#include "af.h"

enum sort_type {ATTACKER_COUNT, VICTIM_COUNT, VICTIMS_DIVIDED_BY_ATTACKERS, ATTACKERS_DIVIDED_BY_VICTIMS, VICTIMS_MINUS_ATTACKERS};
enum sort_direction { SORT_ASCENDING, SORT_DESCENDING };

int *sort_af(AF *af, AF *s_af, enum sort_type criterion, enum sort_direction direction);

// BitSet *map_indices_back(BitSet *s, int *mapping);

// returns the original index (the index before sorting) of the argument
int map_argument_back(int argument, int *mapping);

// returns the mapped index (the index after sorting) of the argument
int map_argument(int argument, int af_size, int *mapping);

#endif //SORT_H
