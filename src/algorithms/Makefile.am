AUTOMAKE_OPTIONS	= subdir-objects
noinst_LIBRARIES   	= lib_algorithms.a

lib_algorithms_a_SOURCES 	= next-closure/stable.c next-closure/stable.h norris/stable.c norris/stable_bu.c norris/stable.h  nourine/stable.c nourine/stable.h nourine/implications.c nourine/implications.h connected-components/wcc.c connected-components/wcc.h connected-components/scc.c connected-components/scc_count.c connected-components/scc.h connected-components/cc.c connected-components/cc.h

lib_algorithms_a_LIBADD		= ../af/af.o ../bitset/bitset.o ../bitset/bitset_hash_map.o ../utils/linked_list.o ../utils/list.o

CLEANFILES      	= *.o
DISTCLEANFILES  	= .deps Makefile