#ifndef __INTERFACES_H__
#define __INTERFACES_H__

#include <stdint.h>
#include <stddef.h>

typedef struct interface_t interface_t;

void clear_interfaces(void);

char * interface_name_get(interface_t * const interface);
void interface_name_set(interface_t * const interface, char const * const if_name, size_t const interface_name_length);

uint16_t interface_link_type_get(interface_t * const interface);
void interface_link_type_set(interface_t * const interface, uint16_t const link_type);

uint32_t interface_snap_length_get(interface_t * const interface);
void interface_snap_length_set(interface_t * const interface, uint32_t const snap_length);


interface_t * interface_get_next_free(void);
interface_t * interface_get_by_index(uint32_t const index);

#if defined(UNIT_TEST)
void unit_test_free(void * const __ptr);
#define INTERFACES_FREE unit_test_free
#else
#define INTERFACES_FREE free
#endif


#endif /* __INTERFACES_H__ */
