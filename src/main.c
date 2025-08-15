#include <vos.h>
#include <vos_route.h>
#include <vos_html.h>
#include <vos_requests.h>

#include <stdio.h>
#include <string.h>

static char *host = "0.0.0.0";
static int port = 8080;

static vos_client* g_client = NULL;
static vos_request_t g_req = {
  .method = "",
  .path = "",
  .http_version = "",
  .body_length = 0,
  .body = NULL,
};

static char* remove_root_slash(char* path) {
  size_t len = strlen(path);

  if (len == 1 && path[0] == '/') {
    char* copy = malloc(2);
    if (!copy) return NULL;
    strcpy(copy, "/");
    return copy;
  }

  char* npath = malloc(len + 1);
  if (!npath) return NULL;
  
  if (path[0] == '/') {
    strcpy(npath, path + 1);
  } else {
    strcpy(npath, path);
  }

  return npath;
}

void mreqhandler(vos_manager* mgr, int client_fd, vos_request_t* request) {
  vos_client* client = vos_search_client(NULL, client_fd);
  
  if (strstr(request->path, ".css")) {
    char* npath = remove_root_slash(request->path);
    vos_display_css(client, npath);
    printf("Loaded: %s\n", npath);
    free(npath);
  } else if (strstr(request->path, ".js")) {
    char* npath = remove_root_slash(request->path);
    vos_display_js(client, npath);
    printf("Loaded: %s\n", npath);
    free(npath);
  } else if (strstr(request->path, "/")) {
    vos_display_html(client, "site/html/index.html");
    printf("Loaded: %s\n", request->path);
  }
}

void meventloop(vos_manager *mgr, int client_fd) { // Add hot reload
  if (g_client == NULL) g_client = vos_search_client(NULL, client_fd); vos_handle_request(g_client, &g_req, mreqhandler);

  vos_client* client = vos_search_client(NULL, client_fd);
}

int main() {
  vos_manager mgr = {
    .host=host,
    .port=port
  };

  if (vos_initialize(&mgr) == 0) {
    vos_start(meventloop);
  }

  vos_stop();
  vos_delete_all();
  return 0;
}
