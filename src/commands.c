#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minizip/zip.h>

#include "../includes/defs.h"

void handle_help(char **args, int argc){
    if (argc != 0){ 
        _handle_too_many_args(**args,argc);
        return;
    }
    printf(GREEN "These are some FileFlow commands:\n\n" RESET);
    printf("  cmp <input_path> <output_path>     Compresses the file at <input_path> and saves to <output_path>\n");
    printf("  dcmp <input_path> <output_path>    Decompresses the ZIP file at <input_path> to <output_path>\n");
    printf("  version                            See the current Version\n");
    printf("  exit                               Exit the system\n\n");
}

void handle_unknown(char *arg){
    printf("%s:" RED " command not found" RESET ", digit help to see the commands\n", arg);
}

void handle_exit(char **args, int argc){
    if (argc != 0){ 
        _handle_too_many_args(**args,argc);
        return;
    }
    printf("Bye Bye\n");
    exit(0);
}

void handle_compress(char **args, int argc){    
    if (argc != 2){
        printf("Usage: cmp <input_path> <output_path>");
        return;
    }
}

void handle_decompress(char **args, int argc){  

}

void handle_version(char **args, int argc){ 
    if (argc != 0){ 
        _handle_too_many_args(**args,argc);
        return;
    }
    printf("%d\n",VERSION);
}

void _handle_too_many_args(char **args,int argc){ 
    printf("Extra token(s): ");
    for (int i = 0;i < argc;i++){   
        printf("%s ", args[i]);
    }
    printf(RED"\n are not recognizable\n"RESET);
}