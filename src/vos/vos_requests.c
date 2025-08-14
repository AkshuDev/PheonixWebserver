#include <vos.h>
#include <vos_requests.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void vos_handle_request(vos_client* client, vos_request_t* req, void* request_handler(vos_manager* mgr, int client_fd, vos_request_t* request)) {
    vos_rq_handler_t *rq = (vos_rq_handler_t*)malloc(sizeof(vos_rq_handler_t));
    rq->handler = request_handler;
    rq->request_updater = vos_update_request;
    rq->request = req;

    client->rq_handler = rq;
}

void vos_send_response(vos_client* client, vos_response_t* response) {
    vos_send(client, response->headers, response->body);
}

void vos_update_request(vos_client* client, vos_request_t* request, char* data) {
    if (data == NULL) data = vos_read(client);

    if (data == NULL) {printf("No data found...\n"); return;}

    char* headers_end = strstr(data, "\r\n\r\n");
    int headers_len = headers_end - data;
    char* body_start = headers_end + 4;

    char* line_end = strstr(data, "\r\n");
    char req_line[512];
    int line_len = line_end - data;
    strncpy(req_line, data, line_len + 1);
    req_line[line_len] = '\0';

    // Extract data
    sscanf(req_line, "%7s %255s %15s", request->method, request->path, request->http_version);

    // Parse content len
    char* cl_header = strstr(data, "Content-Length:"); 
    int content_len = 0;
    if (cl_header) {
        sscanf(cl_header, "Content-Length: %d", &content_len);
    }

    // Store body
    request->body_length = content_len;
    if (content_len > 0) {
        if (request->body) free(request->body);
        request->body = malloc(content_len + 1);
        if (!request->body) request->body_length = 0; return;
        memcpy(request->body, body_start, content_len);
        request->body[content_len] = '\0';
    } else {
        if (request->body) free(request->body);
        request->body = NULL;
    }

    return;
}

void vos_clear_request(vos_request_t* request) {
    request->body_length = 0;
    strcpy(request->method, "");
    strcpy(request->path, "");
    strcpy(request->http_version, "");
    if (request->body) free(request->body);
    request->body = NULL;
}