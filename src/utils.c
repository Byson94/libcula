#include "libcula/utils.h"

void cula_list_init(cula_list *list) {
    list->prev = list;
    list->next = list;
}

static inline void __cula_list_insert(cula_list *list, cula_list *elm) {
    elm->prev = list;
    elm->next = list->next;
    list->next->prev = elm;
    list->next = elm;
}

void cula_list_insert(cula_list *list, cula_list *elm) {
    __cula_list_insert(list, elm);
}

void cula_list_insert_list(cula_list *list, cula_list *other) {
    if (cula_list_empty(other)) {
        return;
    }
    cula_list *prev = other->prev;
    cula_list *next = other->next;

    UNUSED(prev);

    other->prev->next = list->next;
    list->next->prev = other->prev;

    list->next = next;
    next->prev = list;

    cula_list_init(other);
}

void cula_list_remove(cula_list *elm) {
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
    elm->prev = NULL;
    elm->next = NULL;
}

int cula_list_length(const cula_list *list) {
    const cula_list *e;
    int count = 0;

    e = list->next;
    while (e != list) {
        count++;
        e = e->next;
    }

    return count;
}

bool cula_list_empty(const cula_list *list) {
    return list->next == list;
}

void cula_signal_init(cula_signal *signal) {
    cula_list_init(&signal->listener_list);
}

void cula_signal_add(cula_signal *signal, cula_listener *listener) {
    cula_list_insert(signal->listener_list.prev, &listener->link);
}

void cula_signal_emit(cula_signal *signal, void *data) {
    cula_listener *l, *next;
    cula_list_for_each_safe(l, next, &signal->listener_list, link) {
        if (l->notify) {
            l->notify(l, data);
        }
    }
}
