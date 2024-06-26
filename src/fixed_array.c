#include <fixed_array.h>
#include <stdlib.h>
#include <stdio.h>

void _free_nothing(void * v){
    v = v;
}

fixray *fixray_init(uint64_t size){
    fixray * fa = malloc(sizeof(*fa));
    fa->size = size;
    fa->container = malloc(sizeof(*fa->container) * size);
    fa->available_indices = stack_init(size);
    // Fills the stack with the indices
    for (uint64_t i = 0; i < size; i++){
        stack_push(fa->available_indices, (void*)(size - i - 1));
        fa->container[i] = _fixray_null;
    }

    fa->element_to_index = htb_init(size);
    pthread_mutex_init(&fa->mutex, NULL);

    return fa;
}

uint64_t fixray_add(fixray * fa, void * element){
    pthread_mutex_lock(&fa->mutex);
    if (stack_is_empty(fa->available_indices)){
        fprintf(stderr, "Tried to insert into an already full fixed array !\n");
        pthread_mutex_unlock(&fa->mutex);
        return UINT64_MAX;
    }
    if (element == _fixray_null){
        fprintf(stderr, "Tried to insert an element that has the same value as the null used ! Actually unlucky :o \n");
        pthread_mutex_unlock(&fa->mutex);
        return UINT64_MAX;
    }

    uint64_t index = (uint64_t)stack_pop(fa->available_indices);
    htb_add(fa->element_to_index, (int64_t)element, (void*)index);
    fa->container[index] = element;
    fa->count++;
    pthread_mutex_unlock(&fa->mutex);
    return index;
}

uint64_t fixray_get_index(fixray * fa, void * element){
    return (uint64_t)htb_get(fa->element_to_index, (int64_t)element);
}

void fixray_remove_from_index(fixray * fa, uint64_t index){
    pthread_mutex_lock(&fa->mutex);
    void * element = fa->container[index];
    fa->container[index] = _fixray_null;
    stack_push(fa->available_indices, (void*)index);
    htb_remove(fa->element_to_index, (int64_t)element);
    fa->count--;
    pthread_mutex_unlock(&fa->mutex);
}

void fixray_remove_element(fixray * fa, void * element){
    uint64_t index = (uint64_t)htb_get(fa->element_to_index, (int64_t)element);
    fixray_remove_from_index(fa, index);
}

bool fixray_exist(fixray *fa, void* element){
    return htb_exist(fa->element_to_index, (int64_t)element);
}

void fixray_cleanup(fixray * fa){
    stack_cleanup(fa->available_indices);
    free(fa->container);
    htb_cleanup(fa->element_to_index, _free_nothing);
    pthread_mutex_destroy(&fa->mutex);
    free(fa);
}