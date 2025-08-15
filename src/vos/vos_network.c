#include <vos.h>
#include <vos_network.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

static bool is_private_ip_v4(uint32_t be_addr) {
    // be_addr is network byte order
    uint32_t a = ntohl(be_addr);

    if ((a & 0xff000000u) == 0x0a000000u) return true; // 10.0.0.0/8
    if ((a & 0xfff00000u) == 0xac100000u) return true; // 172.16.0.0/12
    if ((a & 0xffff0000u) == 0xc0a80000u) return true; // 192.168.0.0/16
    if ((a & 0xff000000u) == 0x7f000000u) return true; // 127.0.0.0/8
    if ((a & 0xffff0000u) == 0xa9fe0000u) return true; // 169.254.0.0/16
    if ((a & 0xf0000000u) == 0xe0000000u) return true; // 224.0.0.0/4 multicast
    return false;
}

static int parse_http_url(const char* u, vos_net_url_t* out) {
    memset(out, 0, sizeof(*out));
    const char* p = strstr(u, "://");

    if (!p) return -1; // not url

    size_t sch_len = p - u;
    if (sch_len >= sizeof(out->scheme)) return -1;

    memcpy(out->scheme, u, sch_len);
    if (strcmp(out->scheme, "https") != 0) return -1; // not https (secure)

    const char* hostbeg = p + 3;
    const char* pathbeg = strchr(hostbeg, '/');
    const char* hostend = pathbeg ? pathbeg : u + strlen(u);

    const char* colon = memchr(hostbeg, ':', hostend - hostbeg);
    if (colon) {
        size_t hlen = colon - hostbeg;
        if (hlen == 0 || hlen >= sizeof(out->host)) return -1;

        memcpy(out->host, hostbeg, hlen);
        out->port = atoi(colon + 1);

        if (out->port <= 0 || out->port > 65535) return -1;
    } else {
        size_t hlen = hostend - hostbeg;
        if (hlen == 0 || hlen >= sizeof(out->host)) return -1;

        memcpy(out->host, hostbeg, hlen);
        out->port = 443;
    }

    if (pathbeg) {
        size_t plen = strlen(pathbeg);
        if (plen >= sizeof(out->path_qs)) return -1;
        memcpy(out->path_qs, pathbeg, plen);
    } else {
        strcpy(out->path_qs, "/");
    }

    return 0;
}