#ifndef HOSTNAME_UTILS_H
#define HOSTNAME_UTILS_H

#include "../includes/defs.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define PATH_SEPARATOR '\\'
    #define MKDIR(path) _mkdir(path)
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define PATH_SEPARATOR '/'
    #define MKDIR(path) mkdir(path, 0700)
#endif

int get_computer_name(char *buffer, size_t size);
void get_config_path(char *buffer, size_t size);

#endif 
