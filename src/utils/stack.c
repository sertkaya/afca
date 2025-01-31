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
#include <assert.h>

#include "stack.h"

// extern inline void push(Stack*s, unsigned int e);
extern inline void push(Stack*s, StackElement* e);
// extern inline unsigned int pop(Stack* s);
extern inline unsigned int pop_int(Stack* s);
extern inline void* pop_ptr(Stack* s);

extern inline StackElement* new_stack_element_int(int i);
extern inline StackElement* new_stack_element_ptr(void* p);

void init_stack(Stack* s) {
	s->size = 0;
	s->elements = NULL;
}

