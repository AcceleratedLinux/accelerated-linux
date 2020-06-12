#include "interfaces.h"
#include "pcapng.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>

struct interface_t
{
    char * name;
    uint32_t link_type;
    uint32_t snap_length;
};

static interface_t interfaces[MAX_PCAPNG_INTERFACES];
static size_t interface_count;

static void clear_interface(interface_t * const interface)
{
    INTERFACES_FREE(interface->name);
    interface->name = NULL;
    interface->link_type = 0;
    interface->snap_length = 0;
}

void clear_interfaces(void)
{
    size_t index;

    for (index = 0; index < interface_count; index++)
    {
        clear_interface(&interfaces[index]);
    }

    interface_count = 0;
}

void interface_name_set(interface_t * const interface, char const * const interface_name, size_t const interface_name_length)
{
    interface->name = (char*) malloc(interface_name_length + 1);
    
    strncpy(interface->name, interface_name, interface_name_length);
    interface->name[interface_name_length] = '\0';
}

void interface_link_type_set(interface_t * const interface, uint16_t const link_type)
{
    interface->link_type = link_type;
}

void interface_snap_length_set(interface_t * const interface, uint32_t const snap_length)
{
    interface->snap_length = snap_length;
}

uint16_t interface_link_type_get(interface_t * const interface)
{
    return interface->link_type;
}

uint32_t interface_snap_length_get(interface_t * const interface)
{
    return interface->snap_length;
}

char * interface_name_get(interface_t * const interface)
{
    return interface->name;
}

interface_t * interface_get_next_free(void)
{
    interface_t * interface;

    if (interface_count == MAX_PCAPNG_INTERFACES)
    {
        ERROR("Interface limit reached (%zu)\n", interface_count);
        interface = NULL;
        goto done;
    }
    interface = &interfaces[interface_count];
    interface_count++;

done:
    return interface;
}

interface_t * interface_get_by_index(uint32_t const index)
{
    interface_t * interface;

    if (index >= interface_count)
    {
        interface = NULL;
        goto done;
    }
    interface = &interfaces[index];

done:
    return interface;
}
