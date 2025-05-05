#include <stdio.h>
#include <stdlib.h>

void handle_help(char **args, int argc){
    if (argc != 0){ 
        printf("Extra token(s): ");
        for (int i = 0;i < argc;i++){   
            printf("%s ", args[i]);
        }
        printf("\n are not recognizable\n");
        return;
    }
    printf("Blablbalba\n");
}

void handle_unknown(char *arg){
    printf("%s: command not found\n", arg);
}

void handle_exit(char **args, int argc){
    if (argc != 0){ 
        printf("Extra token(s): ");
        for (int i = 0;i < argc;i++){   
            printf("%s ", args[i]);
        }
        printf("\n are not recognizable\n");
        return;
    }
    printf("Bye Bye\n");
    exit(0);
}