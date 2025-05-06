#ifndef HOSTNAME_UTILS_H
#define HOSTNAME_UTILS_H

#include "../includes/defs.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <direct.h>
    #define PATH_SEPARATOR '\\'
    #define MKDIR(path) _mkdir(path)
    #define CLEAR "cls"
    #define GET_HOME() (getenv("USERPROFILE"))

    static char* _get_cwd() {
        static char buffer[1024];
        if (_getcwd(buffer, sizeof(buffer)-2) == NULL) {
            perror("Cannot obtain the curren directory (Windows)");
            return NULL;
        }
        size_t len = strlen(buffer);
        buffer[len] = '\n';
        buffer[len + 1] = '\0';
        return buffer;
    }

#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define PATH_SEPARATOR '/'
    #define MKDIR(path) mkdir(path, 0700)
    #define CLEAR "clear"
    #define GET_HOME() (getenv("HOME"))

    static char* _get_cwd() {
        static char buffer[1024];
        if (getcwd(buffer, sizeof(buffer)-2) == NULL) {
            perror("Cannot obtain the current directory (Unix)");
            return NULL;
        }
        size_t len = strlen(buffer);
        buffer[len] = '\n';
        buffer[len + 1] = '\0';
        return buffer;
    }

#endif

int get_computer_name(char *buffer, size_t size);
void get_config_path(char *buffer, size_t size);

#endif 
