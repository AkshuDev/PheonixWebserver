#include <vos.h>
#include <vos_requests.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

static int server_fd = -1;
static struct sockaddr_in server_addr;

static vos_manager *g_mangr;

int vos_initialize(vos_manager *mangr) {
  int port = mangr->port;
  char *host = mangr->host;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("Socket Creation failed");
    return -1;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
    perror("Invalid Address");
    return -2;
  }

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
      perror("Bind Failed");
      return -3;
  }

  if (listen(server_fd, 10) < 0) {
    perror("Listen Failed");
    return -4;
  }

  g_mangr = mangr;
  
  printf("VOS Server Started on %s:%d\n", host, port);
  return 0;
}

void vos_start(void (*callback)(vos_manager* mgr, int client_fd)) {
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  while (1) {
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
      perror("Accept failed!");
      continue;
    }

    // HTTP read
    char buffer[VOS_MAX_READ];
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
      buffer[bytes_read] = '\0';
      printf("Request found!:\n%s\n", buffer);
      printf("Searching for Client: %d\n", client_fd);
      if (vos_search_client(NULL, client_fd) == NULL) {
        printf("Client Not found, Adding Client...\n");
        vos_client nclient = {
          .location="/",
          .label="/",
          .type="home",
          .gp=NULL,
          .client_fd=client_fd,
          .next=NULL,
          .tail=NULL,
          .clients=NULL,
          .client_count=0,
        };
          
        // Add home dir
        g_mangr->client_count += 1;
        g_mangr->clients = &nclient;
        g_mangr->next = g_mangr->clients;
        g_mangr->tail = g_mangr->clients; // This is the start and the end

        printf("Client added\n");
      }
    }

    vos_client* client = vos_search_client(NULL, client_fd);

    if (client) callback(g_mangr, client_fd);
    
    if (client != NULL && client->rq_handler) {
      // Send data
      vos_rq_handler_t* rq = client->rq_handler;
      if (bytes_read > 0){
        rq->request_updater(client, rq->request, buffer);
      } else {
        rq->request_updater(client, rq->request, NULL);
      }
      rq->handler(g_mangr, client_fd, rq->request);
    }

    close(client_fd);
  }
}

void vos_stop() {
  if (server_fd >= 0) close(server_fd);
}

void vos_delete_clients(vos_client* client) {
  vos_client* cur = client->clients;

  while (cur) {
    if (cur->rq_handler) free(cur->rq_handler);
    if (cur->client_count > 0) vos_delete_clients(cur);
    
    cur->clients = NULL;
    cur->tail = NULL;
    cur->gp = NULL;
    cur->label = NULL;
    cur->location = NULL;
    cur->type = NULL;
    cur->client_count = 0;
    cur->client_fd = 0;

    cur = cur->next;
  }
}

void vos_delete_all() {
  vos_client* cur = g_mangr->next;

  while(cur) {
    if (cur->rq_handler) free(cur->rq_handler);
    if (cur->client_count > 0) vos_delete_clients(cur);
    
    cur->clients = NULL;
    cur->tail = NULL;
    cur->gp = NULL;
    cur->label = NULL;
    cur->location = NULL;
    cur->type = NULL;
    cur->client_count = 0;
    cur->client_fd = 0;

    cur = cur->next;
  }

  g_mangr->client_count = 0;
  g_mangr->clients = NULL;
  g_mangr->next = NULL;
  g_mangr->tail = NULL;
  g_mangr->host = NULL;
  g_mangr->port = 0;
}

void vos_send_x(vos_client *client, const char *header, int header_len, const char *data, int data_len) {
  send(client->client_fd, header, header_len, 0);
  send(client->client_fd, data, data_len, 0);
}

void vos_send_header_x(vos_client *client, const char *header, int header_len) {
  send(client->client_fd, header, header_len, 0);
}

void vos_send(vos_client *client, const char *header, const char *data) {
  send(client->client_fd, header, strlen(header), 0);
  send(client->client_fd, data, strlen(data), 0);
}

void vos_send_header(vos_client *client, const char *header) {
  send(client->client_fd, header, strlen(header), 0);
}

void vos_print_sp(vos_client *client, const char *data, const char *content_type, int status_code) {
 char header[512];
  snprintf(header, sizeof(header),
	   "HTTP/1.1 %zu OK\r\n"
	   "Content-Type: %s\r\n"
	   "Content-Length: %zu\r\n"
	   "Connection: close\r\n"
     "Cache-Control: no-store, no-cache, must-revalidate, max-age=0\r\n"
     "Pragma: no-cache\r\n"
     "Expires: 0\r\n\r\n",
	   status_code, content_type, strlen(data));
  send(client->client_fd, header, strlen(header), 0);
  send(client->client_fd, data, strlen(data), 0);
} 

void vos_print_x(vos_client *client, const char *data, int data_len, const char *content_type) {
  char header[512];
  snprintf(header, sizeof(header),
	   "HTTP/1.1 200 OK\r\n"
	   "Content-Type: %s\r\n"
	   "Content-Length: %zu\r\n"
	   "Connection: close\r\n"
     "Cache-Control: no-store, no-cache, must-revalidate, max-age=0\r\n"
     "Pragma: no-cache\r\n"
     "Expires: 0\r\n\r\n",
	   content_type, data_len);
  send(client->client_fd, header, strlen(header), 0);
  send(client->client_fd, data, data_len, 0);
}
  
void vos_print(vos_client *client, const char *data, const char *content_type) {
  char header[512];
  snprintf(header, sizeof(header),
	   "HTTP/1.1 200 OK\r\n"
	   "Content-Type: %s\r\n"
	   "Content-Length: %zu\r\n"
	   "Connection: close\r\n"
     "Cache-Control: no-store, no-cache, must-revalidate, max-age=0\r\n"
     "Pragma: no-cache\r\n"
     "Expires: 0\r\n\r\n",
	   content_type, strlen(data));
  send(client->client_fd, header, strlen(header), 0);
  send(client->client_fd, data, strlen(data), 0);
}

void vos_print_html(vos_client *client, const char *data) {
  vos_print(client, data, "text/html");
}

void vos_print_css(vos_client *client, const char *data) {
  vos_print(client, data, "text/css");
}

void vos_print_js(vos_client *client, const char *data) {
  vos_print(client, data, "text/js");
}

void vos_print_text(vos_client *client, const char *data) {
  vos_print(client, data, "text/plain");
}

char* vos_read(vos_client* client) {
  char buf[VOS_MAX_READ];
  int bytes_read;

  // Read request
  bytes_read = read(client->client_fd, buf, sizeof(buf) - 1);
  if (bytes_read < 0) {
    perror("[ERROR] Unable to read data from client!\n");
    return NULL;
  }

  buf[bytes_read] = '\0';

  printf("Request: %s\n", buf);

  return buf; // Done!
}

void vos_add_client(vos_client *main_client, vos_client *client) {
  if (!main_client) {
    if (!client) return;

    client->next = NULL; // No linking client

    // Append client to main server
    vos_client *cur = g_mangr->clients;
    while (cur->next) {
      cur = cur->next;
    }
    cur->next = client;

    g_mangr->client_count++;
    g_mangr->tail = client;
    return;
  }

  if (!main_client || !client) return;

  client->next = NULL;
  if (!main_client->clients) {
    // First client
    main_client->clients = client;
    main_client->client_count++;
    main_client->tail = client;
  } else {
    // Append
    vos_client *cur = main_client->clients;
    while (cur->next) {
      cur = cur->next;
    }
    cur->next = client;
    main_client->client_count++;
    main_client->tail = client;
  }
}  

int vos_remove_client(vos_client *main_client, vos_client *client) {
   if (!main_client) {
     if (!client) return -1;

     vos_client *prev = NULL;
     vos_client *cur = g_mangr->clients;
     while (cur) {
       if (cur == client) {
	 if (prev) prev->next = cur->next;
	 else g_mangr->clients = cur->next;

	 if (g_mangr->tail == cur) {
	   g_mangr->tail = prev;
	 }

	 g_mangr->client_count--;

	 return 0;
       }
       prev = cur;
       cur = cur->next;
     }
     return -1;
   }

   if (!main_client || !client) return -1;

   vos_client *prev = NULL;
   vos_client *cur = main_client->clients;

   while(cur) {
     if (cur == client) {
       if (prev) prev->next = cur->next;
       else main_client->clients = cur->next;

       if (main_client->tail == cur) {
	 main_client->tail = prev;
       }

       main_client->client_count--;
       return 0;
     }
     prev = cur;
     cur = cur->next;
   }
   return -1;
}

vos_client* vos_search_client(vos_client *main_client, int client_fd) {
  if (!main_client) {
    vos_client *cur = g_mangr->clients;

    while (cur) {
      if (cur->client_fd == client_fd) {
	return cur;
      }
      cur = cur->next;
    }

    return NULL;
  }

  if (!main_client) return NULL;

  vos_client *cur = main_client->clients;

  while (cur) {
    if (cur->client_fd == client_fd) {
      return cur;
    }

    cur = cur->next;
  }

  return NULL;
}
