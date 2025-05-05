#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/defs.h"
#include "commands.h"
#include "utils.h"

typedef void (*CommandFunc)(char **args, int argc);

typedef struct {
    const char *name;
    CommandFunc func;
} Command;

void freeTokens(char** tokens){ 
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

void processInput(char userInput[]){
    char **tokens = split(userInput);

    if (tokens[0] == NULL) {
        return; 
    }

    Command commands[] = {
        {"help", handle_help},
        {"exit", handle_exit},
        {NULL, NULL} 
    };

    int found = 0;

    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(tokens[0], commands[i].name) == 0) {
            commands[i].func(&tokens[1], count_args(&tokens[1]));
            found = 1;
            break;
        }
    }

    if (!found) {
        handle_unknown(tokens[0]);
    }

    freeTokens(tokens);
} 