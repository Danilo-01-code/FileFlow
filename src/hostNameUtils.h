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
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define PATH_SEPARATOR '/'
    #define MKDIR(path) mkdir(path, 0700)
    #define CLEAR "clear"
    #define GET_HOME() (getenv("HOME"))
#endif

int get_computer_name(char *buffer, size_t size);
void get_config_path(char *buffer, size_t size);

#endif 
