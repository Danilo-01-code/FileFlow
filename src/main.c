#include <stdio.h>
#include <string.h>
#include "../includes/defs.h"
#include "hostNameUtils.h"
#include "utils.h"

int main(){
    char hostname[1024];

    if (!(get_computer_name(hostname, sizeof(hostname)) == 0)) {
        printf("Cannot obtain the computer name\n");
        return 1;
    } 
    char path[512];
    char username[100];
    FILE *file;
    
    get_config_path(path, sizeof(path));
    file = fopen(path,"r");

    if (file){  
        if (fgets(username, sizeof(username), file)){   
            username[strcspn(username, "\n")] = '\0';
        }
        fclose(file);
    }
    else{
        printf(RED"Username not found\n"RESET"Digit your username: ");
        if (fgets(username, sizeof(username), stdin) == NULL) {
            fprintf(stderr, RED "Cannot read input.\n" RESET);
            return 1;
        }
        username[strcspn(username, "\n")] = '\0';
        ensure_config_dir_exists(path);
        file = fopen(path, "w");

        if (!file){ 
            perror(RED "Cannot create a configuration file\n" RESET);
            return 1;
        }
        fprintf(file, "%s\n", username);
        fclose(file);
        printf("Username " BLUE "%s" RESET " saved successfully\n", username);
    }
    printf("%s@%s\n", username, hostname);

    return 0;
}