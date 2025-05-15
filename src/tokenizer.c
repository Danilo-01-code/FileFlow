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

Command commands[] = {
    { "help",    handle_help             },
    { "bye",     handle_bye              },
    { "version", handle_version          },
    { "cmp",     handle_compress         },
    { "dcmp",    handle_decompress       },
    { "clear",   handle_clear            },
    { "curr",    handle_curr_directory   },
    { "arc",     handle_architecture     },
    { "name",    handle_name             },
    { "mv",      handle_move             },
    { "welcome", handle_welcome          },
    { "cd",      handle_change_directory },
    { "mk",      handle_make             },
    { "rm",      handle_remove           },
    { "ls",      handle_ls               },
    { "queue",   handle_queue            },
    { "rn",      handle_rename           },
    { "find",    handle_find             },
    { "read",    handle_read             },
    { "write",   handle_write            },
    {NULL, NULL} 
};

void freeTokens(char** tokens){ 
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

void executeCommand(char** tokens){
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

void processInput(char userInput[], size_t length){
    char **tokens = split(userInput, length);

    if (tokens == NULL) {
        return; 
    }

    executeCommand(tokens);
} 