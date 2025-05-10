AUTOMAKE_OPTIONS	= subdir-objects
noinst_LIBRARIES   	= lib_algorithms.a

lib_algorithms_a_SOURCES 	= cbo/complete.h cbo/complete.c cbo/admissible.c

lib_algorithms_a_LIBADD		= ../utils/linked_list.o ../utils/array_list.o ../utils/stack.o ../utils/priority_queue.o

CLEANFILES      	= *.o
DISTCLEANFILES  	= .deps Makefile