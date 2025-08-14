#pragma once

#ifndef VOS_REQUESTS_H
#define VOS_REQUESTS_H
#endif

#include <vos.h>

void vos_handle_request(vos_client* client, vos_request_t* req, void* request_handler(vos_manager* mgr, int client_fd, vos_request_t* request)); // If it gets a request, it auto sends all data to the request handler
void vos_send_response(vos_client* client, vos_response_t* response);
void vos_update_request(vos_client* client, vos_request_t* request, char* data); // Updates the provided request
void vos_clear_request(vos_request_t* request);