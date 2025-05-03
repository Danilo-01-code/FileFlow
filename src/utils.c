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
    // This function considers that the sentence arg is a mutable string, like: char sentence[]
    int size = count_words(sentence);
    char **tokens = malloc(size * sizeof(char*));
    int token_idx = 0;
    char *token = strtok(sentence, " ");
    
    while (token != NULL){    
        tokens[token_idx++] = strdup(token);
        token = strtok(NULL," ");
    }
    return tokens;
}

int count_words(char *sentence){ 
    int res = 1; // For the first word, this considers that there is no space before the first word
    int len = strlen(sentence);

    for (int i = 0; i < len;i++){   
        if (sentence[i] == ' ' && i < len - 1 && sentence[i+1] != ' '){    
            res++;
        }
    }
    return res;
}