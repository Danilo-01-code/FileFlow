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
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "../includes/defs.h"
#include "hostNameUtils.h"
#include "tokenizer.h"
#include "utils.h"

void showWelcome(void);
int isFirstRunToday(void);
int getUsername(char *username, size_t size, const char *path);
int welcomeEveryRun(void);
void cannot_exit(int signo);

char *prompt = NULL;
char *userInput = NULL;

int main(void){
    char hostname[1024];
    char path[512];
    char username[100];

    if (get_computer_name(hostname, sizeof(hostname)) != 0) {
        printf("Cannot obtain the computer name\n");
        return 1;
    } 

    get_config_path(path, sizeof(path));

    if (getUsername(username, sizeof(username), path) != 0) {
        printf("Cannot obtain or save your username.\n");
        return 1;  
    }

    system(CLEAR);

    if (welcomeEveryRun()){   
        showWelcome();
    }
    else if (!isFirstRunToday()) {
        showWelcome();
    }

    size_t required_size = strlen(BRED) + strlen(username) 
        + strlen(" @ ") + strlen(BGREEN) + strlen(hostname) 
        + strlen(" > ") + strlen(RESET) + 1;

    prompt = (char *)malloc(required_size);

    if (prompt == NULL) {
        fprintf(stderr, RED "Cannot allocate memory for the prompt.\n" RESET);
        return 1;
    }

    signal(SIGINT, cannot_exit); // Avoid memory Leaks.
    
    while(1){   
        snprintf(prompt, required_size, BRED "%s @ " BGREEN "%s > " RESET, username, hostname);
        userInput = readline(prompt);
        to_lowercase(userInput);
    
        if (*userInput) {
            add_history(userInput);
            size_t length_tokens = count_tokens(userInput);
            if (length_tokens > 0) {
                processInput(userInput, length_tokens);
            }
        }
        free(userInput);
    }
    free(prompt);

    return 0;
}

int getUsername(char *username, size_t size, const char *path) {
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
    printf(BBLUE "\n"
        "╔════════════════════════════════════════════════════════════════════╗\n"
        "║      ___________.__.__        ___________.__                       ║\n"
        "║      \\_   _____/|__|  |   ____\\_   _____/|  |   ______  _  __      ║\n"
        "║       |    __)  |  |  | _/ __ \\|    __)  |  |  /  _ \\ \\/ \\/ /      ║\n"
        "║       |     \\   |  |  |_\\  ___/|     \\   |  |_(  <_> )     /       ║\n"
        "║       \\___  /   |__|____/\\___  >___  /   |____/\\____/ \\/\\_/        ║\n"
        "║           \\/                 \\/    \\/                              ║\n"
        "╚════════════════════════════════════════════════════════════════════╝\n\n"
        "                            ,--.    ,--.\n"
        "                           ((O ))--((O ))\n"
        "                         ,'_`--'____`--'_`.\n"
        "                        _:  ____________  :_\n"
        "                       | | ||::::::::::|| | |\n"
        "                       | | ||::::::::::|| | |\n"
        "                       | | ||::::::::::|| | |\n"
        "                       |_| |/__________\\| |_|\n"
        "                         |________________|\n"
        "                      __..-'            `-..__\n"
    RESET);
    printf(BGREEN"                   Your file automation assistant.\n"RESET);
    printf("\n"
        "* Type help to see all available commands.\n"
        "* For more information, check: /docs or README.md\n"
        "This message is shown one time per day. You can change this with the message command.\n\n");
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
    snprintf(path_date, sizeof(path_date), "%s%c.fileflow_last_run", home, PATH_SEPARATOR);

    FILE *f = fopen(path_date, "r+");

    if (f) {
        fgets(last_date, sizeof(last_date), f);
    } else {
        f = fopen(path_date, "w+");
        if (!f) return 0;  
    }

    if (!strcmp(last_date, today)) {
        freopen(path_date, "w", f);
        fprintf(f, "%s\n", today);
        fclose(f);
        return 1;
    }

    fclose(f);
    return 0;
}

int welcomeEveryRun(void){
    char path[512];
    char welcome[8] = {0};
    const char *home = GET_HOME();
    if (!home) home = ".";

    snprintf(path, sizeof(path), "%s%c.welcome", home, PATH_SEPARATOR);
    FILE *f = fopen(path, "r");

    if (!f){    
        return 0;
    }
    
    if (fgets(welcome, sizeof(welcome), f) == NULL) {
        fclose(f);
        return 0;
    }

    welcome[strcspn(welcome, "\n")] = 0;

    if (strcmp(welcome, "0") == 0) {
        fclose(f);
        return 0;
    } else {
        fclose(f);
        return 1;
    }
}

void cannot_exit(int signo){ 
    (void)signo;
    rl_replace_line("", 0);        
    rl_on_new_line();              
    rl_redisplay();                

    printf("\nFor exit use the: " GREEN "'bye'" RESET " command\n");

    rl_on_new_line();              
    rl_redisplay();
}