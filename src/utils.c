#include "libcula/utils.h"

void cula_list_init(cula_list_t *list) {
    list->prev = list;
    list->next = list;
}

static inline void __cula_list_insert(cula_list_t *list, cula_list_t *elm) {
    elm->prev = list;
    elm->next = list->next;
    list->next->prev = elm;
    list->next = elm;
}

void cula_list_insert(cula_list_t *list, cula_list_t *elm) {
    __cula_list_insert(list, elm);
}

void cula_list_insert_list(cula_list_t *list, cula_list_t *other) {
    if (cula_list_empty(other)) {
        return;
    }
    cula_list_t *prev = other->prev;
    cula_list_t *next = other->next;

    UNUSED(prev);

    other->prev->next = list->next;
    list->next->prev = other->prev;

    list->next = next;
    next->prev = list;

    cula_list_init(other);
}

void cula_list_remove(cula_list_t *elm) {
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
    elm->prev = NULL;
    elm->next = NULL;
}

int cula_list_length(const cula_list_t *list) {
    const cula_list_t *e;
    int count = 0;

    e = list->next;
    while (e != list) {
        count++;
        e = e->next;
    }

    return count;
}

bool cula_list_empty(const cula_list_t *list) {
    return list->next == list;
}

void cula_signal_init(cula_signal_t *signal) {
    cula_list_init(&signal->listener_list);
}

void cula_signal_add(cula_signal_t *signal, cula_listener_t *listener) {
    cula_list_insert(signal->listener_list.prev, &listener->link);
}

void cula_signal_emit(cula_signal_t *signal, void *data) {
    cula_listener_t *l, *next;
    cula_list_for_each_safe(l, next, &signal->listener_list, link) {
        if (l->notify) {
            l->notify(l, data);
        }
    }
}
