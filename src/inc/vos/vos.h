#pragma once

#ifndef VOS_H
#define VOS_H
#endif

// Includes
#include <stddef.h>

// Macros
#define VOS_MAX_READ 2048

// Structs
struct vos_client_fd;
struct vos_manager_fd;
typedef struct vos_client_fd vos_client;
typedef struct vos_manager_fd vos_manager;

typedef struct {
  char method[8];
  char path[256];
  char http_version[16];
  char *body;
  int body_length;
} vos_request_t;

typedef struct {
  int status_code;
  char *headers;
  char *body;
} vos_response_t;

typedef struct {
  void (*handler)(vos_manager* mgr, int client_fd, vos_request_t* request);
  void (*request_updater)(vos_client* client, vos_request_t* request, char* data);
  vos_request_t* request;
} vos_rq_handler_t;

struct vos_client_fd {
  char *location;
  char *label;
  char *type;
  char *gp; // General Purpose
  int client_fd;

  struct vos_client *next;
  struct vos_client *tail;
  struct vos_client *clients;

  int client_count;

  vos_rq_handler_t* rq_handler;
};

struct vos_manager_fd {
  char *host;
  int port;
  char* exec_path;
  vos_client *clients;
  vos_client *next;
  vos_client *tail;
  int client_count;
};

// vos.c
int vos_initialize(vos_manager *mangr);

void vos_start(void (*callback)(vos_manager* mgr, int client_fd));
void vos_stop();

void vos_delete_clients(vos_client* client);
void vos_delete_all();

void vos_send(vos_client *client, const char *header, const char *data);
void vos_send_header(vos_client *client, const char *header);
void vos_send_x(vos_client *client, const char *header, int header_len, const char *data, int data_len);
void vos_send_header_x(vos_client *client, const char *header, int header_len);

void vos_print(vos_client *client, const char *data, const char *content_type);
void vos_print_sp(vos_client *client, const char *data, const char *content_type, int status_code);
void vos_print_x(vos_client *client, const char *data, int data_len, const char *content_type);
void vos_print_html(vos_client *client, const char *data);
void vos_print_css(vos_client *client, const char *data);
void vos_print_js(vos_client *client, const char *data);
void vos_print_text(vos_client *client, const char *data);

char* vos_read(vos_client*); // Reads data provided by the client

void vos_add_client(vos_client *main_client, vos_client *client); // if main_client is NULL it uses the global vos_manager instead
int vos_remove_client(vos_client *main_client, vos_client *client);
vos_client* vos_search_client(vos_client *main_client, int client_fd);
