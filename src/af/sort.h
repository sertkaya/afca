//
// Created by bs on 13.09.24.
//

#ifndef SORT_H
#define SORT_H
#include "af.h"

enum sort_type {ATTACKER_COUNT, VICTIM_COUNT, VICTIMS_DIVIDED_BY_ATTACKERS, ATTACKERS_DIVIDED_BY_VICTIMS, VICTIMS_MINUS_ATTACKERS};
enum sort_direction { SORT_ASCENDING, SORT_DESCENDING };

AF* sort_af(AF *af, int sort_type, int sort_direction);

BitSet *map_indices_back(BitSet *s);

#endif //SORT_H
