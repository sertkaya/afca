/*
 * The ELepHant Reasoner
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include "../bitset/bitset.h"
#include "../utils/map.h"
#include "../utils/set.h"

#define MAP_INITIAL_SIZE 	100
#define SET_INITIAL_SIZE 	100
#define BITSET_SIZE			20

int main(int argc, char *argv[]) {

	// create a bit set (a subset of the argument set)
	// it will be the key of the map
	BitSet*  x = create_bitset(BITSET_SIZE);
	// add some arguments to it
	SET_BIT(x, 10);
	SET_BIT(x, 15);
	SET_BIT(x, 80);
	SET_BIT(x, 90);
	SET_BIT(x, 197);
	SET_BIT(x, 198);
	SET_BIT(x, 199);

	// create a set
	Set* s = SET_CREATE(SET_INITIAL_SIZE);
	// create 10 bitsets (subsets of the argument set)
	int i,j;
	for (i = 0; i < 10; ++i) {
		BitSet*  y = create_bitset(BITSET_SIZE);
		// each bitset has 2 arguments
		for (j = 2 * i; j < 2 * i + 2; ++j) {
			SET_BIT(y, j);
		}
		// add the bitset y to the set s
		SET_ADD(y, s);
	}

	// now key is the bitset "x", value is the set of bitsets "s"
	// put the key value pair (x,s) to the map


	// define the map
	Map map;
	// initialize it
	MAP_INIT(&map, MAP_INITIAL_SIZE);
	// put (x,s) to the map
	MAP_PUT(x, s, &map);

	// iterate over the map
	MapIterator map_it;
	MAP_ITERATOR_INIT(&map_it, &map);
	Set* e = MAP_ITERATOR_NEXT(&map_it);
	printf("Iterating .........................:\n");
	int count = 0;
	while (e) {
		printf("value %d:\n", count++);
		// e is the value, in our case the set of bitsets
		// iterate over it and print
		SetIterator* set_it = SET_ITERATOR_CREATE((Set*) e);
		BitSet* bs = SET_ITERATOR_NEXT(set_it);
		while (bs != NULL) {
			print_bitset(bs, stdout);
			printf("\n");
			bs = SET_ITERATOR_NEXT(set_it);
		}
		SET_ITERATOR_FREE(set_it);
		e = MAP_ITERATOR_NEXT(&map_it);
	}


	return 1;
}


