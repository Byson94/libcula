#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <cjson/cJSON.h>

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
            return greetd;
        }
    }

    // Get socket
    const char *socket_path = getenv("GREETD_SOCK");
    if (!socket_path) {
        return NULL;
    }

    struct cula_greetd *greetd = calloc(1, sizeof(struct cula_greetd));
    greetd->user = user;
    greetd->socket = socket_path;

    // Init events
    cula_signal_init(&greetd->events.success);
    cula_signal_init(&greetd->events.error);
    cula_signal_init(&greetd->events.auth_message);

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

int cula_create_session_greetd(struct cula_greetd *greetd) {
    const char *user = greetd->user;
    const char *socket_path = greetd->socket;

    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        free(greetd);
        return 1;
    }
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(socket_fd);
        free(greetd);
        return 1;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "create_session");
    cJSON_AddStringToObject(root, "username", user);
    
    char *payload = cJSON_PrintUnformatted(root);
    if (!payload) {
        cJSON_Delete(root);
        close(socket_fd);
        free(greetd);
        return 1;
    }

    int payload_len = strlen(payload);
    uint32_t net_len = (uint32_t)payload_len;
    
    ssize_t written_len = write(socket_fd, &net_len, sizeof(net_len));
    ssize_t written_payload = write(socket_fd, payload, payload_len);

    // Clean up cJSON objects immediately after use
    cJSON_free(payload);
    cJSON_Delete(root);

    if (written_len != sizeof(net_len) || written_payload != payload_len) {
        close(socket_fd);
        free(greetd);
        return 1;
    }

    greetd->socket_fd = socket_fd;
    return 0;
}

int cula_start_session_greetd(struct cula_greetd *greetd, const char **cmd, const char **env) {
    int socket_fd = greetd->socket_fd;
    if (!socket_fd) return 2;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "start_session");

    int cmd_len = 0;
    if (cmd) {
        while (cmd[cmd_len]) cmd_len++;
    }
    cJSON *cmd_arr = cJSON_CreateStringArray(cmd, cmd_len);
    cJSON_AddItemToObject(root, "cmd", cmd_arr);

    int env_len = 0;
    if (env) {
        while (env[env_len]) env_len++;
    }
    cJSON *env_arr = cJSON_CreateStringArray(env, env_len);
    cJSON_AddItemToObject(root, "env", env_arr);

    char *payload = cJSON_PrintUnformatted(root);
    if (!payload) {
        cJSON_Delete(root);
        return 1;
    }

    int payload_len = strlen(payload);
    uint32_t net_len = (uint32_t)payload_len;

    write(socket_fd, &net_len, sizeof(net_len));
    write(socket_fd, payload, payload_len);

    cJSON_free(payload);
    cJSON_Delete(root);
    return 0;
}

void cula_destroy_greetd(struct cula_greetd *greetd) {
    cula_list_remove(&greetd->service->link);
    free(greetd);
}
