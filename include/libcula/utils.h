/* This file contains code extracted from 'wayland-utils.h'.  
 * (MIT) License of the said header:
 *
 * Copyright © 2008 Kristian Høgsberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifndef CULA_CORE_UTILS_H
#define CULA_CORE_UTILS_H

#include <stdbool.h>
#include <stddef.h>

#define UNUSED(x) (void)(x)

typedef struct cula_list {
    struct cula_list *prev;
    struct cula_list *next;
} cula_list_t;

typedef struct {
    cula_list_t listener_list;
} cula_signal_t;

typedef struct cula_listener cula_listener_t;
struct cula_listener {
    cula_list_t link;
    void (*notify)(cula_listener_t *listener, void *data);
};

#define cula_container_of(ptr, sample, member) \
	(__typeof__(sample))((char *)(ptr) - \
			     offsetof(__typeof__(*sample), member))

/* --- List Management Wrappers (Declarations Only) --- */

void cula_list_init(cula_list_t *list);
void cula_list_insert(cula_list_t *list, cula_list_t *elm);
void cula_list_insert_list(cula_list_t *list, cula_list_t *other);
void cula_list_remove(cula_list_t *elm);
int cula_list_length(const cula_list_t *list);
bool cula_list_empty(const cula_list_t *list);

/* --- Iteration Macros --- */

#define cula_list_for_each(pos, head, member)				\
	for (pos = cula_container_of((head)->next, pos, member);	\
	     &pos->member != (head);					\
	     pos = cula_container_of(pos->member.next, pos, member))

#define cula_list_for_each_safe(pos, tmp, head, member)			\
	for (pos = cula_container_of((head)->next, pos, member),		\
	     tmp = cula_container_of((pos)->member.next, tmp, member);	\
	     &pos->member != (head);					\
	     pos = tmp,							\
	     tmp = cula_container_of(pos->member.next, tmp, member))

#define cula_list_for_each_reverse(pos, head, member)			\
	for (pos = cula_container_of((head)->prev, pos, member);	\
	     &pos->member != (head);					\
	     pos = cula_container_of(pos->member.prev, pos, member))

#define cula_list_for_each_reverse_safe(pos, tmp, head, member)		\
	for (pos = cula_container_of((head)->prev, pos, member),	\
	     tmp = cula_container_of((pos)->member.prev, tmp, member);	\
	     &pos->member != (head);					\
	     pos = tmp,							\
	     tmp = cula_container_of(pos->member.prev, tmp, member))

/* --- Signal Wrappers --- */

void cula_signal_init(cula_signal_t *signal);
void cula_signal_add(cula_signal_t *signal, cula_listener_t *listener);
void cula_signal_emit(cula_signal_t *signal, void *data);

#endif
