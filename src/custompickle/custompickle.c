#include "custompickle.h"
#include "../Automaton.h"


static const char CUSTOMPICKLE_MAGICK[16] = {
    'p', 'y', 'a', 'h', 'o', 'c', 'o', 'r', 'a', 's', 'i', 'c', 'k',    // signature
    '0', '0', '2'                                                       // format version
};


void custompickle_initialize_footer(CustompickleFooter* footer, size_t nodes_count) {

    ASSERT(footer != NULL);

    memcpy(footer->magick, CUSTOMPICKLE_MAGICK, sizeof(CUSTOMPICKLE_MAGICK));
    footer->nodes_count = nodes_count;
}

int custompickle_validate_header(CustompickleHeader* header) {
    if (memcmp(header->magick, CUSTOMPICKLE_MAGICK, sizeof(CUSTOMPICKLE_MAGICK)) != 0)
        return false;

    if (!check_store(header->data.store))
        return false;

    if (!check_kind(header->data.kind))
        return false;

    if (!check_key_type(header->data.key_type))
        return false;

    return true;
}


int custompickle_validate_footer(CustompickleFooter* footer) {
    return (memcmp(footer->magick, CUSTOMPICKLE_MAGICK, sizeof(CUSTOMPICKLE_MAGICK)) == 0);
}
