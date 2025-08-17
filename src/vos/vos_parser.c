#include <vos.h>
#include <vos_parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

vos_executable_t* vos_parse_exec(vos_manager* mangr) {
    FILE* f = fopen(mangr->exec_path, "rb");

    if (!f) {
        perror("Unable to read executable VOS file!\n");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);

    char* buffer = malloc(file_size);
    fread(buffer, file_size, 1, f);

    vos_executable_t* exe = (vos_executable_t*)malloc(sizeof(vos_executable_t));
    if (!exe) {
        perror("Memory ALLOC FAILED!\n");
        return NULL;
    }

    strncpy(exe->header, buffer, 3);
    if (strncmp(buffer, "VOS", 3) != 0) {
        perror("Not a VOS exe file!\n");
        free(exe);
        return NULL;
    }

    int curOff = 3; // Current offset in buffer (starts with 0)
    strncpy(exe->programName, buffer + curOff, 64);
    curOff += 64;
    strncpy(exe->programDescription, buffer + curOff, 256);
    curOff += 256;
    char val;
    memcpy(val, buffer + curOff, 1);
    exe->HttpsOnly = val == 1 ? true : false;

    curOff += 1;
    // Now we parse the buffers
    uint64_t browserOff, osOff, connOff, fileOff;
    uint32_t browserEntries, osEntries, connEntries, fileEntries;
    
    memcpy(browserOff, buffer + curOff, 8);
    memcpy(browserEntries, buffer + curOff + 8, 4);
    curOff += 12;
    memcpy(osOff, buffer + curOff, 8);
    memcpy(osEntries, buffer + curOff + 8, 4);
    curOff += 12;
    memcpy(connOff, buffer + curOff, 8);
    memcpy(connEntries, buffer + curOff + 8, 4);
    curOff += 12;
    memcpy(fileOff, buffer + curOff, 8);
    memcpy(fileEntries, buffer + curOff + 8, 4);

    if (browserOff == VOS_EXEC_NULL) {
        exe->supportedBrowsers = NULL;
    } else {
        vos_exec_browser_t* browsers = (vos_exec_browser_t*)malloc(sizeof(vos_exec_browser_t));
        if (!browsers) {
            perror("Memory Alloc Issue\n");
            return NULL;
        }
        browsers->NUMentries = browserEntries;
        browsers->entries = malloc(browsers->NUMentries * sizeof(vos_exec_browser_entry));
        curOff = browserOff;

        for (int i = 0; i < browserEntries; i++) {
            memcpy(browsers->entries[i].browser, buffer + curOff, 256);
            curOff += 256;
            memcpy(browsers->entries[i].browserLen, buffer + curOff, 4);
            curOff += 8;
        }

        exe->supportedBrowsers = browsers;
    }

    if (connOff == VOS_EXEC_NULL) {
        exe->conns = NULL;
    } else {
        vos_exec_conn_t* conns = (vos_exec_conn_t*)malloc(sizeof(vos_exec_conn_t));
        if (!conns) {
            perror("Memory Alloc Issue\n");
            return NULL;
        }
        conns->NUMentries = connEntries;
        conns->entries = malloc(conns->NUMentries * sizeof(vos_exec_conn_entry));
        curOff = connOff;

        for (int i = 0; i < connEntries; i++) {
            memcpy(conns->entries[i].conn, buffer + curOff, 256);
            curOff += 256;
            memcpy(conns->entries[i].connLen, buffer + curOff, 4);
            curOff += 8;
        }

        exe->conns = conns;
    }

    if (osOff == VOS_EXEC_NULL) {
        exe->supportedOSes = NULL;
    } else {
        vos_exec_os_t* oses = (vos_exec_os_t*)malloc(sizeof(vos_exec_os_t));
        if (!oses) {
            perror("Memory Alloc Issue\n");
            return NULL;
        }
        oses->NUMentries = osEntries;
        oses->entries = malloc(oses->NUMentries * sizeof(vos_exec_os_entry));
        curOff = osOff;

        for (int i = 0; i < osEntries; i++) {
            memcpy(oses->entries[i].os, buffer + curOff, 256);
            curOff += 256;
            memcpy(oses->entries[i].osLen, buffer + curOff, 4);
            curOff += 8;
        }

        exe->supportedOSes = oses;
    }

    if (fileOff == VOS_EXEC_NULL) {
        exe->files = NULL;
    } else {
        vos_exec_files_t* files = (vos_exec_files_t*)malloc(sizeof(vos_exec_files_t));
        if (!files) {
            perror("Memory Alloc Issue\n");
            return NULL;
        }
        files->NUMentries = fileEntries;
        files->entries = malloc(files->NUMentries * sizeof(vos_exec_files_entry));
        curOff = fileOff;

        for (int i = 0; i < fileEntries; i++) {
            memcpy(files->entries[i].file, buffer + curOff, 256);
            curOff += 256;
            memcpy(files->entries[i].fileLen, buffer + curOff, 4);
            curOff += 8;
        }

        exe->files = files;
    }

    free(buffer);

    return exe;
}

void vos_free_exec(vos_executable_t* exec) {
    if (exec->supportedBrowsers != NULL) free(exec->supportedBrowsers->entries);
    if (exec->supportedOSes != NULL) free(exec->supportedOSes->entries);
    if (exec->conns != NULL) free(exec->conns->entries);
    if (exec->files != NULL) free(exec->files->entries);

    if (exec->supportedBrowsers != NULL) free(exec->supportedBrowsers);
    if (exec->supportedOSes != NULL) free(exec->supportedOSes);
    if (exec->conns != NULL) free(exec->conns);
    if (exec->files != NULL) free(exec->files);

    if (exec != NULL) free(exec);
}

void vos_build_executable(vos_executable_t* exe, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        perror("Failed to open file for building VOS exe!\n");
        return;
    }

    
    // ---------------------------
    // Calculate offsets
    // ---------------------------
    uint64_t baseOffset = 3 + 64 + 256 + 1 + (8 * 4) + (4 * 4); // after header + metadata
    uint64_t boff = VOS_EXEC_NULL, osoff = VOS_EXEC_NULL, coff = VOS_EXEC_NULL, foff = VOS_EXEC_NULL;

    if (exe->supportedBrowsers != NULL) {
        boff = baseOffset;
        baseOffset += exe->supportedBrowsers->NUMentries * (256 + 4);
    }

    if (exe->supportedOSes != NULL) {
        osoff = baseOffset;
        baseOffset += exe->supportedOSes->NUMentries * (256 + 4);
    }

    if (exe->conns != NULL) {
        coff = baseOffset;
        baseOffset += exe->conns->NUMentries * (256 + 4);
    }

    if (exe->files != NULL) {
        foff = baseOffset;
        baseOffset += exe->files->NUMentries * (256 + 4);
    }

    // ---------------------------
    // Write header and metadata
    // ---------------------------
    fwrite(exe->header, 3, 1, f);
    fwrite(exe->programName, 64, 1, f);
    fwrite(exe->programDescription, 256, 1, f);

    uint8_t val = exe->HttpsOnly ? 1 : 0;
    fwrite(&val, 1, 1, f);

    // Write offsets + entry counts
    fwrite(&boff, 8, 1, f);  
    uint32_t be = exe->supportedBrowsers ? exe->supportedBrowsers->NUMentries : 0;
    fwrite(&be, 4, 1, f);

    fwrite(&osoff, 8, 1, f);
    uint32_t oe = exe->supportedOSes ? exe->supportedOSes->NUMentries : 0;
    fwrite(&oe, 4, 1, f);

    fwrite(&coff, 8, 1, f);
    uint32_t ce = exe->conns ? exe->conns->NUMentries : 0;
    fwrite(&ce, 4, 1, f);

    fwrite(&foff, 8, 1, f);
    uint32_t fe = exe->files ? exe->files->NUMentries : 0;
    fwrite(&fe, 4, 1, f);

    // ---------------------------
    // Write entries (buffers)
    // ---------------------------

    if (exe->supportedBrowsers != NULL) {
        for (int i = 0; i < exe->supportedBrowsers->NUMentries; i++) {
            fwrite(exe->supportedBrowsers->entries[i].browser, 256, 1, f);
            fwrite(&exe->supportedBrowsers->entries[i].browserLen, 4, 1, f);
        }
    }

    if (exe->supportedOSes != NULL) {
        for (int i = 0; i < exe->supportedOSes->NUMentries; i++) {
            fwrite(exe->supportedOSes->entries[i].os, 256, 1, f);
            fwrite(&exe->supportedOSes->entries[i].osLen, 4, 1, f);
        }
    }

    if (exe->conns != NULL) {
        for (int i = 0; i < exe->conns->NUMentries; i++) {
            fwrite(exe->conns->entries[i].conn, 256, 1, f);
            fwrite(&exe->conns->entries[i].connLen, 4, 1, f);
        }
    }

    if (exe->files != NULL) {
        for (int i = 0; i < exe->files->NUMentries; i++) {
            fwrite(exe->files->entries[i].file, 256, 1, f);
            fwrite(&exe->files->entries[i].fileLen, 4, 1, f);
        }
    }

    fclose(f);
}