#pragma once

#ifndef VOS_HTML_H
#define VOS_HTML_H
#endif

#include <vos.h>

char* vos_display_html(vos_client *client, const char *html_path);
char* vos_display_css(vos_client* client, const char* css_path);
char* vos_display_js(vos_client* client, const char* js_path);
