/*
* FileFlow - main.c
* Author: Danilo Kepler
* Created 2025-05-01
* Version: 1.0
*
* Description:
*     Entry-point For FileFlow utility, handles username, hostname
*     and creates an REPL for fileFlow commands. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../includes/defs.h"
#include "hostNameUtils.h"
#include "tokenizer.h"
#include "utils.h"

void showWelcome(void);
int isFirstRunToday(void);
int get_username(char *username, size_t size, const char *path);

int main(void){
    char hostname[1024];
    char path[512];
    char username[100];

    if (get_computer_name(hostname, sizeof(hostname)) != 0) {
        printf("Cannot obtain the computer name\n");
        return 1;
    } 

    get_config_path(path, sizeof(path));

    if (get_username(username, sizeof(username), path) != 0) {
        printf("Cannot obtain or save your username.\n");
        return 1;  
    }

    system(CLEAR);

    if (isFirstRunToday()) {
        showWelcome();
    }

    char userInput[256];
    
    while(1){   
        printf(BRED "%s @ " BGREEN "%s> " RESET, username, hostname);
        fgets(userInput,sizeof(userInput),stdin);
        
        size_t length = count_words(userInput);
        if (length > 0){
            processInput(userInput, length);      
        }
    }

    return 0;
}

int get_username(char *username, size_t size, const char *path) {
    FILE *file = fopen(path, "r");

    if (file) {
        if (fgets(username, size, file)) {
            username[strcspn(username, "\n")] = '\0'; 
        }
        fclose(file);
        return 0; 
    } else {
        printf(RED "Username not found\n" RESET "Please enter your username: ");
        if (fgets(username, size, stdin) == NULL) {
            fprintf(stderr, RED "Cannot read input.\n" RESET);
            return 1; 
        }
        username[strcspn(username, "\n")] = '\0'; 

        ensure_config_dir_exists(path);
        file = fopen(path, "w");
        if (!file) {
            perror(RED "Cannot create a configuration file\n" RESET);
            return 1;  
        }

        fprintf(file, "%s\n", username);
        fclose(file);
        printf("Username " BLUE "%s" RESET " saved successfully\n", username);
        return 0;  
    }
}

void showWelcome(void){ 
    printf(BOLD BBLUE "\n"
        "╔════════════════════════════════════════════════════════════╗\n"
        "║                      Welcome to FileFlow                  ║\n"
        "╚════════════════════════════════════════════════════════════╝\n" RESET);
    printf(BGREEN"           Your file automation assistant.\n"RESET);
    printf("\n* Type help to see all available commands.\n");
    printf("* For more information, check: /docs or README.md\n");
    printf("This message is show one time per day.\n\n");
}

int isFirstRunToday(void) {
    char path_date[512];
    char last_date[11] = {0};
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char today[11];

    strftime(today, sizeof(today), "%Y-%m-%d", tm_info);

    const char *home = GET_HOME();
    if (!home) home = ".";
    snprintf(path_date, sizeof(path_date),
             "%s%c.fileflow_last_run",
             home, PATH_SEPARATOR);

    FILE *f = fopen(path_date, "r+");

    if (f) {
        fgets(last_date, sizeof(last_date), f);
    } else {
        f = fopen(path_date, "w+");
        if (!f) return 0;  
    }

    if (strcmp(last_date, today) != 0) {
        freopen(path_date, "w", f);
        fprintf(f, "%s\n", today);
        fclose(f);
        return 1;
    }

    fclose(f);
    return 0;
}