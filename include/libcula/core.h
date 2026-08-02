#pragma once

#ifndef CULA_CORE_H
#define CULA_CORE_H

#include <wayland-util.h>

struct cula_context;

struct cula_context *cula_create_context(void);
void cula_destroy_context(struct cula_context *ctx);
int cula_run_context(struct cula_context *ctx);

#endif
