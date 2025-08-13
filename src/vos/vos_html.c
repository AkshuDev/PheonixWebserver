#include <vos_html.h>

#include <stdlib.h>
#include <stdio.h>

char* vos_display_html(vos_client *client, const char *html_path) {
  // Open the file
  FILE *fd = fopen(html_path, "r");
  if (!fd) {
    vos_print_sp(client, "404 Not Found : HTML file could not be opened.", "text/plain", 404);
    perror("[ERROR] Could not find HTML file -> vos_display_html\n");
    return;
  }

  fseek(fd, 0, SEEK_END);
  long size = ftell(fd);
  rewind(fd);

  char *buffer = malloc(size + 1);
  if (!buffer) {
    fclose(fd);
    vos_print_sp(client, "500 Internal Server Error : Memory allocation failed.", "text/plain", 500);
    perror("[ERROR] Could not allocate memory for HTML file -> vos_display_html\n");
    return;
  }

  fread(buffer, 1, size, fd);
  buffer[size] = '\0';

  vos_print_html(client, buffer);
  return buffer;
}
