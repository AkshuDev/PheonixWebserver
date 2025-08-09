#pragma once

#ifndef VOS_ROUTE_H
#define VOS_ROUTE_H
#endif

#include <stddef.h>
#include <stdint.h>

// vos_route.c
vos_client *vos_match_request(vos_manager *mgr, const char *path);
