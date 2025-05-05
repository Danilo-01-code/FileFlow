#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "hostNameUtils.h"

void ensure_config_dir_exists(const char *path) {
    char dir_path[512];
    strncpy(dir_path, path, sizeof(dir_path));
    char *last_sep = strrchr(dir_path, PATH_SEPARATOR);
    if (last_sep) *last_sep = '\0';
    MKDIR(dir_path); 
}

char **split(char *sentence){ 
    size_t len = strlen(sentence);
    if (len > 0 && sentence[len - 1] == '\n') {
        sentence[len - 1] = '\0';
    }

    // This function considers that the sentence arg is a mutable string, like: char sentence[]
    int size = count_words(sentence);
    char **tokens = malloc((size+1) * sizeof(char*));
    int token_idx = 0;
    char *token = strtok(sentence, " ");
    
    while (token != NULL){    
        tokens[token_idx++] = strdup(token);
        token = strtok(NULL," ");
    }

    tokens[token_idx] = NULL; // Terminator
    return tokens;
}

int count_words(char *sentence){ 
    int idx = 0;
    while (sentence[idx] == ' '){   
        idx++;
    }
    int res = 1;
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