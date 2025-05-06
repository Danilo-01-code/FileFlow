#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "utils.h"
#include "hostNameUtils.h"

void ensure_config_dir_exists(const char *path) {
    char dir_path[512];
    strncpy(dir_path, path, sizeof(dir_path));
    char *last_sep = strrchr(dir_path, PATH_SEPARATOR);
    if (last_sep) *last_sep = '\0';
    MKDIR(dir_path); 
}

char **split(char *sentence, size_t wordLen){ 
    size_t totalLen = strlen(sentence);
    if (totalLen > 0 && sentence[totalLen - 1] == '\n') {
        sentence[totalLen - 1] = '\0';
    }

    // This function considers that the sentence arg is a mutable string, like: char sentence[]
    char **tokens = malloc((wordLen+1) * sizeof(char*));
    int token_idx = 0;
    char *token = strtok(sentence, " ");
    
    while (token != NULL){    
        tokens[token_idx++] = strdup(token);
        token = strtok(NULL," ");
    }

    tokens[token_idx] = NULL; // Terminator
    return tokens;
}

size_t count_words(char *sentence){ 
    int idx = 0;
    while (sentence[idx] == ' '){   
        idx++;
    }

    if (strlen(sentence) == 0){ 
        return 0;
    }
    size_t res = 1;
    int len = strlen(sentence);

    for (idx; idx < len; idx++){   
        if (sentence[idx] == ' ' && idx < len - 1 && sentence[idx+1] != ' '){    
            res++;
        }
    }
    return res;
}

int count_args(char **args) {
    int count = 0;
    while (args[count] != NULL) {
        count++;
    }
    return count;
}

int dir_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int create_dir(const char *path) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);

    if (tmp[len - 1] == '/')
        tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            MKDIR(tmp);
            *p = '/';
        }
    }
    return MKDIR(tmp);
}