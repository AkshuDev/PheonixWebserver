#pragma once

#ifndef VOS_NETWORK_H
#define VOS_NETWORK_H
#endif

#include <vos.h>
#include <stdbool.h>

typedef struct {
    const char* host;
    bool allow_https_only;
    size_t max_resp_bytes; // cap response size
    int allow_get;
    int allow_post;
} vos_net_policy_entry_t;

typedef struct {
    const vos_net_policy_entry_t* entries;
    size_t count;
} vos_net_policy_t;

typedef struct {
    char scheme[8]; // ex - https
    char host[256];
    int port; // default = 443 for https
    char path_qs[1024]; // path + query
} vos_net_url_t;

typedef struct {
    int socket;
} vos_net_tls_conn_t;

int handle_vos_fetch(vos_client* client, const vos_request_t* req, const vos_net_policy_t* policy);