#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

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

void remove_dir(const char *path){
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        printf("Cannot open dir\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        printf(RED "Deleting dir" RESET"-> %s\n", fullpath);

        struct stat statbuf;
        if (stat(fullpath, &statbuf) != 0) {
            perror("Cannot access file");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            remove_dir(fullpath); // recursive dir remove
        } else {
            if (remove(fullpath) != 0)
                perror("Cannot remove file");
        }
    }

    closedir(dir);

    if (rmdir(path) != 0) {
        printf("Cannot remove dir by the root\n");
        return;
    }

}

char *ensure_zip_extension(const char *filename){
    size_t len = strlen(filename);
    
    if (len >= 4 && strcmp(filename + len - 4, ".zip") == 0) {
        return strdup(filename); 
    }

    char *new_name = malloc(len + 5); 
    if (!new_name) {
        fprintf(stderr, "Cannot allocate memory\n");
        exit(1);
    }

    sprintf(new_name, "%s.zip", filename);
    return new_name;
}

char* make_absolute_dir(const char *path) {
    char *abs_path = NULL;
    char *cwd = _get_curr();

    if (!cwd) return strdup("");  // secure Fallback 

    if (!path || strcmp(path, ".") == 0) {
        abs_path = strdup(cwd);
    } else if (path[0] == '/') { // already absolute
        abs_path = strdup(path);
    } else {
        size_t n = strlen(cwd) + 1 + strlen(path) + 1; 
        abs_path = malloc(n);
        if (abs_path) snprintf(abs_path, n, "%s/%s", cwd, path);
    }

    if (!abs_path) return strdup("");

    struct stat st;
    if (stat(abs_path, &st) != 0) {
        if (MKDIR(abs_path) != 0) {
            perror("Failed to create directory");
            free(abs_path);
            return strdup("");
        }
    } else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Path exists but is not a directory: %s\n", abs_path);
        free(abs_path);
        return strdup("");
    }

    return abs_path;
}

char* make_absolute_file(const char *path) {
    char *abs_path = NULL;
    char *cwd = _get_curr();

    if (!cwd) return strdup("");

    if (!path) {
        abs_path = strdup(cwd);
    } else if (path[0] == '/') {
        abs_path = strdup(path);
    } else {
        size_t n = strlen(cwd) + 1 + strlen(path) + 1;
        abs_path = malloc(n);
        if (abs_path) snprintf(abs_path, n, "%s/%s", cwd, path);
    }
    if (!abs_path) return strdup("");
    return abs_path;
}

int is_subdir(const char *parent, const char *child) {
    size_t len = strlen(parent);
    return strncmp(parent, child, len) == 0 && (child[len] == '/' || child[len] == '\0');
}

// returns 0 if there is no file and -1 if cannot access
long long get_directory_size(const char *dir_path) {
    long long size = 0;
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", dir_path);

    hFind = FindFirstFile(search_path, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        perror("FindFirstFile");
        return -1;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0)
            continue;
        
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s\\%s", dir_path, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            size += get_directory_size(path); // Recursion on dirs
        } else {
            size += ((long long)findFileData.nFileSizeHigh << 32) | findFileData.nFileSizeLow;
        }
        
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

#else
    DIR *dir = opendir(dir_path);

    if (dir == NULL) {
        perror("Cannot open the dir");
        return -1;
    }

    struct dirent *entry;
    struct stat file_stat;
    char path[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (stat(path, &file_stat) == -1){   
            perror("stat");
            continue;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            size += get_directory_size(path); // recursion on dirs
        } else {
            size += file_stat.st_size; 
        }
    }

    closedir(dir);
#endif
    return size;
}

void cleanup_and_exit(char **prompt, char **userInput, int exit_code) {
    if (userInput && *userInput) {
        free(*userInput);
        *userInput = NULL;
    }
    if (prompt && *prompt) {
        free(*prompt);
        *prompt = NULL;
    }
    printf("Bye Bye\n");
    exit(exit_code);
}

void to_lowercase(char *str) {
    while (*str) {
        *str = tolower((unsigned char)*str);
        str++;
    }
}

long get_file_size(const char* file) {
    FILE *f = fopen(file, "r");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    return size;
}