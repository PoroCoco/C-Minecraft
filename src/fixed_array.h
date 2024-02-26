#pragma once

#include <stack.h>
#include <hashtable.h>
#include <stdint.h>

#define _fixray_null ((void*)0x0B00B135)

/*
A fixed array is supposed to hold a static amount of elements. Elements presence/absence can quickly be determined, futhermore they will keep the same position in the container as long as they are not removed.
Element can be removed either from the index given back after the add operation, or by giving the element itself.
Used here to keep the chunks at the same position so they don't get reuploaded to the gpu when new chunks are acquired.
*/
typedef struct fixray{
    stack * available_indices;
    htb * element_to_index; //keys: pointer, values: index 
    void ** container; 
    uint64_t size;
    uint64_t count;
} fixray;

fixray *fixray_init(uint64_t size);

uint64_t fixray_get_index(fixray * fa, void * element);
uint64_t fixray_add(fixray * fa, void * element);
void fixray_remove_from_index(fixray * fa, uint64_t index);
void fixray_remove_element(fixray * fa, void * element);
bool fixray_exist(fixray *fa, void* element);

void fixray_cleanup(fixray * fa, void free_element(void*));