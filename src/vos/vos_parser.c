#include <vos.h>
#include <vos_parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* readString(FILE* file) {
    // Read until \0 or 0x00
    char res[512];

    char val;
    int offset = 0;
    fread(val, sizeof(char), 1, file);
    res[offset] = val;
    offset++;

    while (val != '\0') {
        fread(val, sizeof(char), 1, file);
        res[offset] = val;
        offset++;
    }

    return res;
}

vos_executable_t vos_parse_exec(vos_manager* mangr) {
    vos_executable_t execStruct = {
        .header = NULL,
        .programName = "Unknown",
        .programDescription = "Unknown"
    };

    FILE* exec = fopen(mangr->exec_path, "rb");
    
    if (!exec) {
        perror("Unable to open executable VOs file!\n");
        return execStruct;
    }

    fseek(exec, 0, SEEK_END);
    size_t size = ftell(exec);
    if (size < 38) { // Min size
        perror("Wrong file!\n");
        return execStruct;
    }
    rewind(exec);

    fread(execStruct.header, sizeof(char), 3, exec);
    if (execStruct.header[0] != 'V' || execStruct.header[1] != 'O' || execStruct.header[2] != 'S') {
        perror("Wrong file!: Header doesn't match!\n");
        return execStruct;
    }

    execStruct.programName = readString(exec);
    execStruct.programDescription = readString(exec);
    fread(execStruct.HttpsOnly, sizeof(bool), 1, exec);
    fread(execStruct.supportedBrowsers, sizeof(vos_exec_browser_t), 1, exec);
    fread(execStruct.supportedOSes, sizeof(vos_exec_os_t*), 1, exec);
    fread(execStruct.conns, sizeof(vos_exec_conn_t*), 1, exec);
    fread(execStruct.files, sizeof(vos_exec_files_t*), 1, exec);

    return execStruct;
}