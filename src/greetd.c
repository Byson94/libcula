#include <stdlib.h>
#include <string.h>

#include "libcula/services/greetd.h"
#include "internal/core.h"
#include "libcula/utils.h"

static void destroy_greetd_instruction(cula_listener_t *listener, void *data) {
    UNUSED(data);
    struct cula_greetd *greetd = cula_container_of(listener, greetd, CULA_INTERNAL.destroy);
    cula_destroy_greetd(greetd);
}

struct cula_greetd *cula_get_or_create_greetd(struct cula_context *ctx, const char *user) {
    const char *greetd_id = "cula.service.greetd";
    struct cula_service *existing;
    cula_list_for_each(existing, &ctx->services, link) {
        if (strcmp(existing->name, greetd_id) == 0) {
            struct cula_greetd *greetd = existing->service_ptr;
            if (greetd) return greetd;
        }
    }

    struct cula_greetd *greetd = calloc(1, sizeof(struct cula_greetd));
    greetd->user = user;

    // Init events
    cula_signal_init(&greetd->events.success);
    cula_signal_init(&greetd->events.error);
    cula_signal_init(&greetd->events.auth_message);

    // Setup greetd session

    // Setup service
    struct cula_service *service = calloc(1, sizeof(struct cula_service));
    service->service_ptr = greetd;
    service->name = greetd_id;
    cula_signal_init(&service->destroy_signal);

    greetd->CULA_INTERNAL.destroy.notify = destroy_greetd_instruction;
    greetd->service = service;

    cula_list_insert(&ctx->services, &service->link);
    cula_signal_add(&service->destroy_signal, &greetd->CULA_INTERNAL.destroy);

    return greetd;
}

void cula_destroy_greetd(struct cula_greetd *greetd) {
    cula_list_remove(&greetd->service->link);
    free(greetd);
}
