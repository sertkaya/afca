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

#include "../utils/array_list.h"

int main(int argc, char *argv[]) {

	ArrayList *l = list_create();

    list_add(0, l);
    list_add(1, l);
    list_add(2, l);
    list_add(3, l);
    list_add(4, l);
    list_add(5, l);
    list_add(8, l);
    list_add(19, l);
    list_add(20, l);
    print_list(stdout, l, "\n");

    ArrayList *c = array_list_complement_sorted(l, 20);
    print_list(stdout, c, "\n");

	return 1;
}


