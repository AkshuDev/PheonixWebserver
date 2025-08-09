#include <vos.h>
#include <vos_route.h>
#include <vos_html.h>

#include <stdio.h>

static char *host = "0.0.0.0";
static int port = 8080;

void meventloop(vos_manager *mgr, int client_fd) {
  vos_display_html(vos_search_client(NULL, client_fd), "website/html/index.html");
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
  return 0;
}
