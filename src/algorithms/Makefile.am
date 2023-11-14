AUTOMAKE_OPTIONS	= subdir-objects
noinst_LIBRARIES   	= lib_algorithms.a

lib_algorithms_a_SOURCES 	= next-closure/stable.c next-closure/stable.h norris/stable.c norris/stable.h  nourine/stable.c nourine/stable.h 

lib_algorithms_a_LIBADD		= ../af/af.o ../bitset/bitset.o ../utils/linked_list.o

CLEANFILES      	= *.o
DISTCLEANFILES  	= .deps Makefile