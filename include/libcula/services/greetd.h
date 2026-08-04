#pragma once

#ifndef CULA_GREETD_H
#define CULA_GREETD_H

#include "libcula/utils.h"

struct cula_context;

/**
 * The authentication type.
 */
enum cula_greetd_auth_type {
    CULA_GREETD_AUTH_VISIBLE,
    CULA_GREETD_AUTH_SECRET,
    CULA_GREETD_AUTH_INFO,
    CULA_GREETD_AUTH_ERROR,
};

/**
 * The authentication message.
 */
struct cula_greetd_auth_msg {
    enum cula_greetd_auth_type type;
    char *message;
};

/**
 * The cula greetd session.
 */
struct cula_greetd {
    struct cula_service *service;
    const char *user;

    struct {
        cula_signal_t success;
        cula_signal_t error;
        cula_signal_t auth_message; // cula_greetd_auth_msg
    } events;

    struct {
        cula_listener_t destroy;
    } CULA_INTERNAL;
};

/**
 * Create the greetd service and a session.
 *
 * @param ctx The running cula context.
 * @param user The user.
 * @return `struct cula_greetd *` The cula greetd session. Can be NULL if not available.
 */
struct cula_greetd *cula_get_or_create_greetd(struct cula_context *ctx, const char *user);

/**
 * Start the greetd service.
 *
 * @param greetd The greetd service.
 * @param cmd Command array to execute upon successful auth.
 * @param env Environment variables array.
 */
void cula_start_session_greetd(struct cula_greetd *greetd, const char **cmd, const char **env);

/**
 * Respond to an authentication message.
 *
 * @param auth_msg The authentication message.
 * @param response The response to submit.
 */
void cula_submit_response_greetd(struct cula_greetd_auth_msg *auth_msg, const char *response);

/**
 * Destroy the cula greetd service and cancel the session.
 *
 * @param greetd The greetd service to cancel.
 */
void cula_destroy_greetd(struct cula_greetd *greetd);

#endif
