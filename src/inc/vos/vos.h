#pragma once

#ifndef VOS_H
#define VOS_H
#endif

// Includes
#include <stddef.h>

// Structs
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
  char *location;
  char *label;
  char *type;
  char *gp; // General Purpose
  int client_fd;

  struct vos_client *next;
  struct vos_client *tail;
  struct vos_client *clients;

  int client_count;
} vos_client;

typedef struct {
  char *host;
  int port;
  vos_client *clients;
  vos_client *next;
  vos_client *tail;
  int client_count;
} vos_manager;

// vos.c
int vos_initialize(vos_manager *mangr);

void vos_start();
void vos_stop();

void vos_send(vos_client *client, const char *header, const char *data);
void vos_send_header(vos_client *client, const char *header);
void vos_send_x(vos_client *client, const char *header, int header_len, const char *data, int data_len);
void vos_send_header_x(vos_client *client, const char *header, int header_len);

void vos_print(vos_client *client, const char *data, const char *content_type);
void vos_print_sp(vos_client *client, const char *data, const char *content_type, int status_code);
void vos_print_x(vos_client *client, const char *data, int data_len, const char *content_type);
void vos_print_html(vos_client *client, const char *data);
void vos_print_text(vos_client *client, const char *data);

void vos_add_client(vos_client *main_client, vos_client *client); // if main_client is NULL it uses the global vos_manager instead
int vos_remove_client(vos_client *main_client, vos_client *client);
vos_client* vos_search_client(vos_client *main_client, int client_fd);
