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

#include "../utils/map.h"
#include "../utils/timer.h"

#define TEST_SIZE 10000
#define MAP_SIZE 100
#define BITSET_SIZE			TEST_SIZE

int main(int argc, char *argv[]) {
	struct timeval start_time, stop_time;
	uint64_t i;
	int test_size = TEST_SIZE;
	int map_size = MAP_SIZE;

	uint64_t* tmp = malloc(test_size * sizeof(uint64_t));
	assert(tmp != NULL);

	Map map;
	MAP_INIT(&map, map_size);
	printf("%d put operations .........................: ", test_size);
	fflush(stdout);

	BitSet*  x = create_bitset(BITSET_SIZE);
	START_TIMER(start_time);
	for (i = 1; i < test_size + 1; ++i) {
		SET_BIT(x, i);
		tmp[i] = i;
		MAP_PUT(x, (void*) tmp[i], &map);
	}
	STOP_TIMER(stop_time);
	printf("%.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);

	MapIterator it;
	MAP_ITERATOR_INIT(&it, &map);
	void* e = MAP_ITERATOR_NEXT(&it);
	printf("Iterating .........................: ");
	fflush(stdout);
	START_TIMER(start_time);
	i = 0;
	while (e) {
		e = MAP_ITERATOR_NEXT(&it);
		++i;
	}
	STOP_TIMER(stop_time);
	printf("done in %.3f milisecs\n", TIME_DIFF(start_time, stop_time) / 1000);
	printf("%lu elements\n", i);


	return 1;
}


