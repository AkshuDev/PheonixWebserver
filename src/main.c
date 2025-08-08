#include <mongoose/mongoose.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define true 1
#define false 0

#define MGSTR(str_literal) {str_literal, sizeof(str_literal) - 1 }

typedef enum {
  FB_DEBUG,
  FB_PRINT,
  FB_HTML,
  FB_LOG,
} FallbackModes;

struct sse_client {
  struct mg_connection *c;
  bool is_sensor_stream;
  struct sse_client *next;
};

static struct sse_client *clients = NULL;

static void add_sse_client(struct mg_connection *c, bool is_sensor_stream) {
  struct sse_client *cli = malloc(sizeof(*cli));
  cli->c = c;
  cli->is_sensor_stream = is_sensor_stream;
  cli->next = clients;
  clients = cli;
}

static void broadcast_sse(const char *event, const char *data, int is_sensor_stream) {
  for (struct sse_client *cli = clients; cli; cli = cli->next) {
    if (cli->is_sensor_stream == is_sensor_stream) {
      mg_printf(cli->c, "event: %s\ndata: %s\n\n", event, data);
    }
  }
}

// ESP Update
static void handle_esp_update(const char* msg) {
  broadcast_sse("sensor_update", msg, 1);
}

char* html_content = NULL;
size_t html_size = 0;
time_t last_mod_time = 0;

static const char* listenon = "http://0.0.0.0:8080"; // Full network port - 8080

static void send_log(struct mg_connection *c, const char* msg) {
  printf("[LOG] Sending Log\n");
  mg_printf(c, "data: %s\n\n", msg);
}

int load_html_file(const char* filename) { // Loads HTML file if it is updated
  struct stat st;
  int stcode = stat(filename, &st);
  
  if (stcode == 0) {
    if (st.st_mtime != last_mod_time) {
      // File is modified
      FILE* f = fopen(filename, "rb");
      if(!f) return 1;

      free(html_content);
      
      html_content = malloc(st.st_size + 1);
      fread(html_content, 1, st.st_size, f);

      html_content[st.st_size] = '\0';

      fclose(f);

      html_size = st.st_size;
      last_mod_time = st.st_mtime;

      printf("Reloaded HTML file!\n");
    }
  } else if (stcode == -1) return 1;

  return 0;
}

char* fallback_mode(FallbackModes mode, const char* msg, const char* heading, bool newline, bool print_mode) { // Log/Print/Debug/Error through Fallback mode incase issue with HTML file
  if (mode == FB_DEBUG) {
    char* fallback_str = "";

    if (print_mode == true) {
      fallback_str = "FALLBACK-DEBUG";
    }
    
    char* final;              ;
    asprintf(&final, "[%s] [%s] : %s", fallback_str, heading, msg);
    
    if (newline == true) {
      strcat(final, "\n");
    }

    return final;
  } else if (mode == FB_PRINT) {
    char* final = msg;

    if (newline == true) {
      strcat(final, "\n");
    }
  } else if (mode == FB_HTML) {
    char* final;
    asprintf(&final, "<html>\n<body>\n<p>[%s] : %s", heading, msg);

    if (newline == true) {
      strcat(final, "<bl>");
    }

    strcat(final, "<\p>\n</body>\n</html>");
    return final;
  } else if (mode == FB_LOG) {
    char* fallback_str = "";

    if (print_mode == true) {
      fallback_str = "FALLBACK-LOG";
    }
    
    char* final;
    asprintf(&final, "[%s] [%s] : %s", fallback_str, heading, msg);

    if (newline == true) {
      strcat(final, "\n");
    }

    return final;
  } else {
    return "";
  }
}

void log_msg(struct mg_mgr* mgr, char* buf) {
  for (struct mg_connection *c = mgr->conns; c != NULL; c = c->next) {
    if (c->data[0] == 1) { // SSE Connection
      send_log(c, buf);
    }
  }
}

static void timer_fn(void* args) {
  struct mg_mgr *mgr = (struct mg_mgr *) args;
  struct mg_connection *conn;
  time_t now = time(NULL);
  char buf[64];
  snprintf(buf, sizeof(buf), "[SERVER] [TIME]: %s", ctime(&now));
  log_msg(mgr, buf);
}

  
void fn(struct mg_connection* c, int ev, void* ev_data) { // Event Handler
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message* msg = (struct mg_http_message*) ev_data;

    struct mg_str uri = msg->uri;
    struct mg_str logs = MGSTR("/logs");
    struct mg_str home = MGSTR("/");
    struct mg_str sensors = MGSTR("/sensors");
    struct mg_str esp = MGSTR("/esp");
    
    int opt = load_html_file("medbot.html");
    
    if (opt == 1) {
      perror("CRITICAL!: Failed to open content file, RESORTING TO FALLBACK\n");
      mg_http_reply(c, 500, "Content-Type: text/plain\r\n", fallback_mode(FB_DEBUG, "Failed to open content file!", "CRITICAL", true, true));
      return;
    }

    if (opt == 2) {
      perror("CRITICAL!: Unable to seek content file, RESORTING TO FALLBACK\n");
      mg_http_reply(c, 500, "Content-Type: text/plain\r\n", fallback_mode(FB_DEBUG, "Failed to seek content file!", "CRITICAL", true, true));
      return;
    }
    
    if (opt == 3) {
      perror("CRITICAL!: Failed to get file size!, RESORTING TO FALLBACK\n");
      mg_http_reply(c, 500, "Content-Type: text/plain\r\n", fallback_mode(FB_DEBUG, "Failed to get file size of content file!", "CRITICAL", true, true));
      return;
    } 
    // Main reply
    if (mg_match(uri, home, NULL)) {
      mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s", html_content);
    } else if (mg_match(uri, logs, NULL)) {
      c->data[0] = 1; // Mark as SSE
      printf("Creating SSE...\n");
      mg_printf(c,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/event-stream\r\n"
		"Cache-Control: no-cache\r\n"
		"Connection: keep-alive\r\n\r\n");
      printf("[LOG] : Sending SSE Creation Log Test\n");
      send_log(c, "[SERVER] [LOG]: Log Stream is [LIVE]\n");
      
    } else if (mg_match(uri, sensors, NULL)) {
      printf("Creating SSE...\n");
      mg_printf(c,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/event-stream\r\n"
		"Cache-Control: no-cache\r\n"
		"Connection: keep-alive\r\n\r\n");
      printf("[LOG] : Sending SSE Creation Log Test\n");
      send_log(c, "%UPDATE%%TEMP%20\n");
    } else if (mg_match(uri, esp, NULL)) {
	char buf[256];
	mg_http_get_var(&msg->body, "data", buf, sizeof(buf));
	handle_esp_update(buf);
	mg_http_reply(c, 200, "", "OK\n");
	printf("[LOG] Handled ESP32-Update on Server, Updates should be implemented.\n");
      }
  }
}

int main(void) { // Main func
  printf("Starting...\n");
  printf("Entering full Debug Mode...\n");

  mg_log_set(MG_LL_DEBUG);
  
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, listenon, fn, &mgr);

  // Start a timer
  mg_timer_add(&mgr, 300000, MG_TIMER_REPEAT, timer_fn, &mgr);
  
  printf("Listening and Running Server on: %s\n", listenon);

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }

  printf("Server Closed\nFreeing Data...\n");
  mg_mgr_free(&mgr);
  printf("Full shutdown complete\n");
  return 0;
}
  
