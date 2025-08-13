#include <vos.h>
#include <vos_route.h>
#include <vos_html.h>

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

static char *host = "0.0.0.0";
static int port = 8080;

static time_t site_mtime;

void meventloop(vos_manager *mgr, int client_fd) { // Add hot reload
  struct stat st;

  if (stat("site/html/index.html", &st) < 0) {
    printf("[Server] [ERROR] Could not load HTML file!\n");
    return; // No file found
  }

  stat("site/html/index.html", &st); // For safety

  if (st.st_mtime != site_mtime) {
    vos_display_html(vos_search_client(NULL, client_fd), "site/html/index.html");
    site_mtime = st.st_mtime;
    printf("[Server] Updated Modified Timings.\n");
  }
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
