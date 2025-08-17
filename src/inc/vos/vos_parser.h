#pragma once

#ifndef VOS_PARSER_H
#define VOS_PARSER_H
#endif

#include <vos.h>
#include <stdbool.h>
#include <stdint.h>

#define VOS_EXEC_NULL 0xFF

struct vos_exec_conn_entry {
    char conn[256];
    int connLen;
};

struct vos_exec_files_entry {
    char file[256];
    int fileLen;
};

struct vos_exec_browser_entry {
    char browser[256];
    int browserLen;
};

struct vos_exec_os_entry {
    char os[256];
    int osLen;
};

typedef struct {
    struct vos_exec_conn_entry entries;
    int NUMentries;
} vos_exec_conn_t;

typedef struct {
    struct vos_exec_files_entry entries;
    int NUMentries;
} vos_exec_files_t;

typedef struct {
    struct vos_exec_browser_entry entries;
    int NUMentries;
} vos_exec_browser_t;

typedef struct {
    struct vos_exec_os_entry entries;
    int NUMentries;
} vos_exec_os_t;

typedef struct {
    char header[3]; // VOS
    char programName[64]; // name of the file
    char programDescription[256]; // Description of the program
    bool HttpsOnly; // Only https connections?
    vos_exec_browser_t* supportedBrowsers; // Supported Browsers 64-bits
    vos_exec_os_t* supportedOSes; // Supported OSes 64-bits
    vos_exec_conn_t* conns; // all the connections done in the program 64-bits
    vos_exec_files_t* files; // all the files in this program 64-bits
} vos_executable_t;

vos_executable_t* vos_parse_exec(vos_manager* mangr);
void vos_free_exec(vos_executable_t* exe);
void vos_build_executable(vos_executable_t* exe);