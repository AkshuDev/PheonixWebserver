#include <vos.h>
#include <vos_route.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define _POSIX_C_SOURCE 200809L

static int str_ends_with(const char *s, const char *suffix) {
  if (!s || !suffix) return 0;
  size_t ls = strlen(s), lt = strlen(suffix);
  return (ls >= lt && strcmp(s + ls - lt, suffix) == 0);
}

static int str_starts_with(const char *path, const char *prefix) {
  if (!path || !prefix) return 0;
  size_t lp = strlen(prefix);
  return (strncmp(path, prefix, lp) == 0);
}

vos_client *vos_match_request(vos_manager *mangr, const char *path) {
  if (!mangr || !path) return NULL;

  vos_client *cur = mangr->clients;

  while (cur) {
    if (cur->label && strcmp(cur->label, path) == 0) return cur;
    cur = cur->next;
  }

  cur = mangr->clients;
  while (cur) {
    if (cur->location && str_ends_with(cur->location, "/*")) {
      size_t ln = strlen(cur->location);
      size_t prefix_len = (ln > 0) ? (ln - 1) : 0;
      if (prefix_len == 0) { cur = cur->next; continue; }
      if (strncmp(cur->location, path, prefix_len) == 0) return cur;
    }
    cur = cur->next;
  }

  return NULL;
}

