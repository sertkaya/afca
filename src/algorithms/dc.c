#include "dc.h"

BitSet* dc(AF* af, SIZE_TYPE a, BitSet* (se)(AF*)) {
    if (CHECK_ARG_ATTACKS_ARG(af, a, a)) {
          return NULL;
    }

    BitSet* arguments = create_bitset(af->size);
    complement_bitset(af->graph[a], arguments);

    BitSet* loops = create_bitset(af->size);
    for (SIZE_TYPE i = 0; i < af->size; ++i) {
        if (CHECK_ARG_ATTACKS_ARG(af, i, a)) {
            SET_BIT(loops, i);
        }
    }

    PAF* paf = project_argumentation_framework_with_loops(af, arguments, loops);
    BitSet* extension = se(paf->af);
    if (extension) {
        BitSet* full_extension = project_back(extension, paf);
        SET_BIT(full_extension, a);
        free_bitset(extension);
        extension = full_extension;
    }

    free_paf(paf, true);
    free_bitset(loops);
    free_bitset(arguments);

    return extension;
}
