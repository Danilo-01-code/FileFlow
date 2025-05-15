#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <minizip/zip.h>
#include <minizip/unzip.h>
#include <sys/types.h>  
#include <sys/wait.h>    
#include <time.h>

#include "../includes/defs.h"
#include "tokenizer.h"
#include "hostNameUtils.h"
#include "utils.h"

char *last_outputs[MAX_OUTPUTS];

void _handle_last_single_output(const char *filename) {
    for (int i = 0; i < MAX_OUTPUTS; i++) {
        if (last_outputs[i] != NULL) {
            free(last_outputs[i]);
        }
    }
    last_outputs[0] = strdup(filename); 
}

void _handle_too_many_args(char **args, int argc){ 
    printf("Extra token(s): ");
    for (int i = 0;i < argc;i++){   
        printf("%s ", args[i]);
    }
    printf(RED"\n are not recognizable\n"RESET);
}

void handle_help(char **args, int argc){
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }

    printf(GREEN "These are some FileFlow commands:\n\n" RESET
        "  cmp   <input_path> <output_path>          Compresses the file at <input_path> and saves on <output_path>\n"
        "  queue <file | dir> [other commands]       Make an command execution queue, see /docs/queue.md for more info\n"
        "  dcmp  <input_path> <output_path>          Decompresses the ZIP file at <input_path> to <output_path>\n"
        "  rm    <dir | file> [dir | file] [...]     Remove files or directories\n"
        "  write <file> <text>                       Write an <text> on an given file\n"
        "  read  <file>                              Read an file\n"
        "  mv    <input_path> <output_path> -c | -x  Copy (-c flag) or Cut (-x flag) the files of a given <input_file> to an <output_file>\n"
        "        -c : Copy mode (the original files remain in place)\n"
        "        -x : Cut mode (the original files are removed after transfer)\n"
        "             by pattern the <output_path> id /out\n\n"
        "  mk   <dir | file>            Make a directory or a file. Write '/example' for create a dir and 'example.extension' for a file\n"
        "  rn   <dir | file> <new_name> Rename a file or directory\n"
        "  find <file> <word>           Finds all occurrences of <word> in a guiven <file>\n"
        "  version             See the current Version\n"
        "  clear               Clear the REPL\n"
        "  curr                See the current directory\n"
        "  arc                 See your computer architecture (64 or 32 bits)\n"
        "  name                Your current OS name\n"
        "  exit                Exit the system\n"
        "  welcome             Change the welcome config\n"
        "  cd                  Change directory\n"
        "  ls [path]           List all directories and files on a given path, if has no parameter them list from the current path\n\n");
}

void handle_unknown(char *arg){
    srand(time(NULL));

    const char* sentences[] = {
        "I cannot recognize this: '%s'\n",
        "I am composed of more than 2000 lines of C code, but i cannot recognize this: '%s'\n",
        "I'm not enough advanced to recognize whatever it is: '%s' means\n",
        "I have no clue of what is an '%s'\n",
        "What a hell is an '%s' ?\n",
        "Bro, just tell me '%s' and hope I understand\n",
        "I cannot help you with: %s\n",
        "I ran your input through 1000000 neural networks and still don't know what '%s' is\n",
        "'%s'? Sounds like something my compiler would reject too\n",
        "Are you sure '%s' isn't a Pokémon?\n",
        "Well, '%s' isn't in my vocabulary, and I'm offended\n",
        "I looked up '%s' in all my functions... still no clue what this supposed to mean\n",
        "I'm smart, but not '%s'-level smart\n",
        "Analyzing... Analyzing... Still don't know what '%s' means\n",
        "I'm fluent in C, sarcasm, and confusion. '%s' fits the last category.\n",
        "You should try to asks for '%s' to chatgpt\n",
    };
    
    int n = sizeof(sentences) / sizeof(sentences[0]);
    int index = rand() % n;

    printf(sentences[index], arg);
}

void handle_bye(char **args, int argc){
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }
    cleanup_and_exit(&prompt, &userInput, 0);
    exit(0);
}

int _handle_compress_file(char *path, zipFile zf, char *relpath) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 1;

    zip_fileinfo zi = {0};
    if (zipOpenNewFileInZip(zf, relpath, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK) {
        fclose(fp);
        return 1;
    }

    char buffer[4096];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        zipWriteInFileInZip(zf, buffer, bytes_read);
    }

    fclose(fp);
    zipCloseFileInZip(zf);
    return 0;
}

int _handle_compress_directory(zipFile zf, const char *dirpath, const char *basepath) {
    DIR *dir = opendir(dirpath);
    if (!dir) return -1;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        size_t path_len = strlen(dirpath) + 1 + strlen(entry->d_name) + 1;
        char *path = malloc(path_len);
        snprintf(path, path_len, "%s/%s", dirpath, entry->d_name);

        struct stat st;
        if (stat(path, &st) != 0) {
            free(path);
            continue;
        }

        size_t relpath_len = strlen(basepath) + 1 + strlen(entry->d_name) + 1;
        char *relpath = malloc(relpath_len);
        snprintf(relpath, relpath_len, "%s/%s", basepath, entry->d_name);

        if (S_ISDIR(st.st_mode)) {
            zip_fileinfo zi = {0};
            char *relpath_slash = malloc(relpath_len + 2);
            snprintf(relpath_slash, relpath_len + 2, "%s/", relpath);
            zipOpenNewFileInZip(zf, relpath_slash, &zi, NULL, 0, NULL, 0, NULL, 0, 0);
            zipCloseFileInZip(zf);
            _handle_compress_directory(zf, path, relpath);
            free(relpath_slash);
        } else if (S_ISREG(st.st_mode)) {
            _handle_compress_file(path, zf, relpath);
        }

        free(path);
        free(relpath);
    }

    closedir(dir);
    return 0;
}

void handle_compress(char **args, int argc) {
    if (argc > 2 || argc == 0) {
        printf("Usage: cmp <input_path> [output_dir]\n");
        return;
    }

    char *in_path;
    struct stat st;

    in_path = make_absolute_file(args[0]); // tries to make an file absolut path 
    if (!in_path || stat(in_path, &st) != 0) {
        free(in_path);

        in_path = make_absolute_dir(args[0]);

        if (!in_path || stat(in_path, &st) != 0) {
            fprintf(stderr, RED "Invalid input path: %s\n" RESET, args[0]);
            free(in_path);
            return;
        }
    }

    char *out_dir = (argc == 2) ? make_absolute_dir(args[1]) : make_absolute_dir(".");

    if (!out_dir) {
        fprintf(stderr, RED "Failed to resolve output directory\n" RESET);
        free(in_path);
        return;
    }

    char *basename = strrchr(in_path, '/'); 
    basename = basename ? basename + 1 : in_path;
    // tries to take the filenam from the path: 
    // example: /home/d/document.txt -> document.txt

    size_t zip_name_len = strlen(basename) + 5; // + ".zip" + '\0'
    char *zip_name = malloc(zip_name_len);
    if (!zip_name) {
        fprintf(stderr, RED "Memory allocation failed\n" RESET);
        free(in_path);
        free(out_dir);
        return;
    }
    snprintf(zip_name, zip_name_len, "%s.zip", basename);

    size_t zip_path_len = strlen(out_dir) + 1 + strlen(zip_name) + 1;
    char *zip_path = malloc(zip_path_len);
    if (!zip_path) {
        fprintf(stderr, RED "Memory allocation failed\n" RESET);
        free(in_path);
        free(out_dir);
        free(zip_name);
        return;
    }
    snprintf(zip_path, zip_path_len, "%s/%s", out_dir, zip_name);

    zipFile zf = zipOpen(zip_path, APPEND_STATUS_CREATE);
    if (!zf) {
        fprintf(stderr, RED "Cannot create zip file: %s\n" RESET, zip_path);
        free(in_path);
        free(out_dir);
        free(zip_name);
        free(zip_path);
        return;
    }

    int ret = 0;
    if (S_ISDIR(st.st_mode)) {
        ret = _handle_compress_directory(zf, in_path, basename);
    } else {
        ret = _handle_compress_file(in_path, zf, basename);
    }

    zipClose(zf, NULL);

    if (ret == 0) {
        printf(GREEN "Zip file created From: " RESET "%s" GREEN " to: " RESET "%s\n", in_path, zip_path);
        _handle_last_single_output(zip_path);
    } else {
        fprintf(stderr, RED "Error compressing: %s\n" RESET, in_path);
    }

    free(zip_name);
    free(out_dir);
    free(zip_path);
}

void handle_decompress(char **args, int argc){  
    if (argc > 2 || argc == 0) {
        printf("Usage: dcmp <input_path> <output_path>\n");
        return;
    }

    char *in_zip  = make_absolute_file(args[0]);
    char *out_dir = (argc == 2) ? make_absolute_dir(args[1]) : make_absolute_dir(".");

    unzFile zip_input = unzOpen(in_zip);
    if (!zip_input) {
        fprintf(stderr, RED "Cannot open zip file: %s\n" RESET "Verify if your put the '.zip' on the input\n", in_zip);
        return;
    }

    do {
        char filename[512];
        unz_file_info info;

        if (unzGetCurrentFileInfo(zip_input, &info, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK) {
            break;
        }

        size_t fn_len = strlen(filename);
        if (fn_len > 0 && filename[fn_len - 1] == '/') {
            char dir_path[1536];
            snprintf(dir_path, sizeof(dir_path), "%s/%s", out_dir, filename);
            create_dir(dir_path);
            continue;
        }

        char full_path[1536];
        snprintf(full_path, sizeof(full_path), "%s/%s", out_dir, filename);

        char parent[1024];
        strncpy(parent, full_path, sizeof(parent));
        char *slash = strrchr(parent, '/');
        if (slash) {
            *slash = '\0';
            create_dir(parent);
        }

        if (unzOpenCurrentFile(zip_input) != UNZ_OK) {
            fprintf(stderr, RED "Failed to open entry: %s\n" RESET, filename);
            break;
        }

        FILE *out = fopen(full_path, "wb");
        if (!out) {
            fprintf(stderr, RED "Failed to create file: %s\n" RESET, full_path);
            
            if (full_path[0] == '/'){ 
                fprintf(stderr, RED "Do not use slash '/' if you isnt accessing a directory from the root: %s\n" RESET, full_path);
            }

            unzCloseCurrentFile(zip_input);
            break;
        }

        char buffer[8192];
        int  bytes;
        while ((bytes = unzReadCurrentFile(zip_input, buffer, sizeof(buffer))) > 0) {
            fwrite(buffer, 1, bytes, out);
        }

        fclose(out);
        unzCloseCurrentFile(zip_input);

        printf(GREEN "Extracted from:"RESET " %s " GREEN "to:" RESET "%s\n",in_zip, full_path);
        _handle_last_single_output(full_path);

    } while (unzGoToNextFile(zip_input) == UNZ_OK);

    unzClose(zip_input);
    free(in_zip);
    free(out_dir);
}


void handle_version(char **args, int argc){ 
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }
    printf("%f.2f\n",VERSION);
}

void handle_clear(char **args, int argc){
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }
    system(CLEAR);
}

void handle_curr_directory(char **args, int argc){
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }  
    char * curr = _get_curr();
    printf("%s\n", curr);
}

void handle_architecture(char **args,int argc){ 
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }
    printf("%s\n", ARC);
}

void handle_name(char **args, int argc){  
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }  
    printf("%s\n", NAME);
}

void handle_welcome(char **args, int argc){
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }  

    char path[512];
    char welcome[8] = {0};
    const char *home = GET_HOME();
    if (!home) home = ".";

    snprintf(path, sizeof(path), "%s%c.welcome", home, PATH_SEPARATOR);
    FILE *f = fopen(path, "r+");

    if (!f) {
        f = fopen(path, "w+");
        if (!f) return;
        strcpy(welcome, "1"); 
        fprintf(f, "1\n");
        fclose(f);
        printf("The " BBLUE "Welcome message" RESET " now will be shown whenever you start the system\n");
        return;
    }

    if (fgets(welcome, sizeof(welcome), f) == NULL) {
        fclose(f);
        return;
    }

    welcome[strcspn(welcome, "\n")] = 0;

    fseek(f, 0, SEEK_SET);
    if (strcmp(welcome, "0") == 0) {
        fprintf(f, "1\n");
        printf("The " BBLUE "Welcome message" RESET " now will be shown whenever you start the system\n");
    } else {
        fprintf(f, "0\n");
        printf("The " BBLUE "Welcome message" RESET " now will " RED "not" RESET " be shown whenever you start the system\n");
    }

    fclose(f);
}

void _handle_copy(const char *in_file, const char *out_file, int move_flag) {
    FILE *in_fp = fopen(in_file, "rb");
    FILE *out_fp = fopen(out_file, "wb");

    if (!in_fp || !out_fp) {
        fprintf(stderr, RED "Cannot open files\n" RESET);
        if (in_fp) fclose(in_fp);
        if (out_fp) fclose(out_fp);
        return;
    }

    char buf[4096]; size_t r;
    while ((r = fread(buf, 1, sizeof(buf), in_fp)) > 0) fwrite(buf,1,r,out_fp);
    fclose(in_fp); fclose(out_fp);

    if (!move_flag) {
        if (remove(in_file) == 0)
            printf(GREEN  "Moved: " RESET "%s -> %s\n", in_file, out_file);
        else fprintf(stderr, RED"Failed to remove original\n" RESET);
    } else {
        printf(GREEN "Copied: " RESET "%s" GREEN "->" RESET "%s\n", in_file, out_file);
    }
}

void _handle_move_dir(const char *in_dir, const char *out_dir, int move_flag) {
    char *abs_in = make_absolute_dir(in_dir); // is necessary make all the dirs absolute
    char *abs_out = make_absolute_dir(out_dir);

    if (!abs_in || !abs_out) {
        free(abs_in);
        free(abs_out);
        return;
    }

    if (is_subdir(abs_in, abs_out)){  
        fprintf(stderr,"Cannot copy/cut on nested directories (avoid infinit recursion). Please choose a different output directory\n");
        return;
    }

    DIR *d = opendir(abs_in);
    if (!d) {
        fprintf(stderr, RED "Cannot open input directory\n" RESET);
        free(abs_in);
        free(abs_out);
        return;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        size_t src_n = strlen(abs_in) + 1 + strlen(ent->d_name) + 1;
        size_t dst_n = strlen(abs_out) + 1 + strlen(ent->d_name) + 1;

        char *src = malloc(src_n);
        char *dst = malloc(dst_n);

        snprintf(src, src_n, "%s/%s", abs_in, ent->d_name);
        snprintf(dst, dst_n, "%s/%s", abs_out, ent->d_name);

        struct stat st;
        if (stat(src, &st) == 0 && S_ISDIR(st.st_mode)) {
            _handle_move_dir(src, dst, move_flag);
        } else {
            _handle_copy(src, dst, move_flag);
        }

        free(src);
        free(dst);
    }

    closedir(d);

    free(abs_in);
    free(abs_out);
}

void handle_move(char **args, int argc) {
    if (argc < 1 || argc > 3) {
        printf("Usage: mv <input_dir> <output_dir> [-c|-x]\n");
        return;
    }
    char *output_dir = (argc == 2) ? make_absolute_dir(args[1]) : make_absolute_dir("out");

    int move_flag = 1; 
    if (argc == 3) {
        if (!strcmp(args[2], "-x")) move_flag = 0;
        else if (!strcmp(args[2], "-c")) move_flag = 1;
        else { fprintf(stderr, "Unknown flag %s\n", args[2]); return; }
    }
    _handle_move_dir(args[0], output_dir, move_flag);
    _handle_last_single_output(output_dir);
    free(output_dir);
}

void handle_change_directory(char **args, int argc){
    if (argc != 1){     
        printf("Usage: cd <dir>\n");
        return;
    }

    if (chdir(args[0]) != 0) {
        fprintf(stderr, "Cannot Change to directory '%s'\n", args[0]);
        return;
    }

    char * curr = _get_curr();

    if (strstr(args[0],"..")){  
        printf("%s" RED " <-\n" RESET, curr);
    }     
    else{   
        printf(GREEN "-> " RESET "%s\n", curr);
    }  

    _handle_last_single_output(curr);
}

void handle_make(char **args, int argc){ 
    if (argc != 1){     
        printf("Usage: mk <dir | file>\n");
        return;
    }

    char *new_file = args[0];

    if (new_file[0] == '/'){  // Assumes that is an directory
        char *dir = strdup(new_file);
        if (dir == NULL) {
            printf("strdup\n");
            return;
        }

        memmove(dir, dir + 1, strlen(dir));

        if (MKDIR(dir) != 0) {
            printf("The path: '%s' already exist\n", dir);
            return;
        }
        printf("directory: '/%s' created\n", dir); 
        free(dir);
        return;
    }

    FILE *f = fopen(new_file,"w"); // Assumes is an file
    if (f == NULL) {
        printf("cannot create the file: '%s'\n",new_file);
        return;
    }
    fclose(f);
    printf("File: '%s' created\n", new_file);
    _handle_last_single_output(new_file);
}   

void _handle_remove_all(char *path){

    struct stat path_stat;
    int    emptyContent = 0;

    if (stat(path, &path_stat) != 0){
        printf(RED "Cannot access this path\n" RESET);
        return;
    }

    if (S_ISREG(path_stat.st_mode)) {
        path = make_absolute_file(path);
        if (remove(path) == 0) printf(RED "Deleting file" RESET "-> '%s'\n", path);
        else printf(RED"Cannot remove the file\n" RESET);
        return;
    }

    path = make_absolute_dir(path);
    long long size = get_directory_size(path); 

    if (size == -1 || path == NULL){
        printf(RED"Cannot verify directory\n"RESET);
        return;
    }

    if (size == 0) {
        if (rmdir(path) == 0){  
            printf(RED "Deleting empty dir " RESET "-> '%s'\n", path);
            return;
        }
        else{   
            emptyContent = 1;
        }
    }

    char userChoice;

    while (1){  
        if (emptyContent){
            printf("The directory "BGREEN "%s" RESET " has empty files and/or directories inside it.\n", path);
        }
        else{
            printf("The directory " BGREEN "%s" RESET " has " BRED "%lld" RESET " bytes of size.\n", path, size);
        }

        printf("Do you want to delete ALL the directory? [Y | N] ");
        scanf(" %c", &userChoice);

        if (userChoice == 'y' || userChoice == 'Y')
        {
            printf("Deleting all the contents of " BGREEN "%s\n" RESET, path);
            remove_dir(path);
            break;
        }
        else if (userChoice == 'n' || userChoice == 'N')
        {
            break;
        }
    }
}

void handle_remove(char **args, int argc){  
    if (argc < 1){     
        printf("Usage: rm <dir | file> [dir |file] [...]\n");
        return;
    }

    for (int i = 0; args[i] != NULL; i++){  
        _handle_remove_all(args[i]);
    }
}

void handle_ls(char **args, int argc){  
    if (argc > 1){ 
        printf("Usage: ls [path]\n");
        return;
    }  
    char* path;

    if (argc == 1){ 
        path = make_absolute_dir(args[0]);
        if (path == NULL){ 
            printf("Cannot open the directory.");
            return;
        }
    }
    else{   
        path = _get_curr(); // By default searchs on the current directory
    }
    
    struct dirent *entry;
    DIR *dir = opendir(path);

    printf("Content of: " BGREEN "%s\n" RESET, path)
    ;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(fullpath, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                printf(GREEN "[DIR]" RESET "   %s\n", entry->d_name);
            }          
            else if (S_ISREG(statbuf.st_mode)) {
                printf(BLUE "[FILE]" RESET "  %s\n", entry->d_name);
            }
        } 
            else {
                perror("Cannot access element");
            }
    }
    closedir(dir);
}

void handle_queue(char **args, int argc) {
    if (argc < 2) {
        printf("queue <file | directory> <command 1> <command 2> [...]\n");
        return;
    }
    char *path = make_absolute_file(args[0]);
    path = path ? path : make_absolute_dir(path);
    
    if (!path){ 
        printf("Cannot recognize: %s\n", path);
        return;
    }

    char** commands = args + 1;       // <commands 1> <commands 2> [...]
    _handle_last_single_output(path); // <file | directory

    for (int i = 0; commands[i] != NULL; i++){ 
        char* current_args[] =  {commands[i], last_outputs[0], NULL}; 
        executeCommand(current_args);
    } 
    free(path);
}

void handle_rename(char **args, int argc){  
    if (argc != 2){ 
        printf("Usage: rn <file | dir> <new_name>");
        return;
    }

    char *new_path = args[1];
    char *old_path = make_absolute_file(args[0]);
    old_path = old_path ? old_path : make_absolute_dir(old_path);
    
    if (!old_path){ 
        printf("Cannot recognize: %s\n", old_path);
        return;
    }

    const char *last_slash = strrchr(old_path, '/'); // last occurrence
    size_t dir_len = last_slash - old_path;

    char absolute_new_path[512];
    snprintf(absolute_new_path, sizeof(absolute_new_path), "%.*s/%s", (int)dir_len, old_path, new_path);

    if (rename(old_path, absolute_new_path) == 0){  
        printf("Path: " GREEN "'%s'" RESET " renamed to: " GREEN "'%s'\n" RESET, old_path, absolute_new_path);
        _handle_last_single_output(absolute_new_path);
    }
    else{   
        printf("Cannot rename Path: " GREEN "'%s'" RESET " to: " GREEN "'%s'\n" RESET, old_path, absolute_new_path);
    }
    free(old_path);    
}

void handle_find(char **args, int argc){ 
    if (argc !=2){  
        printf("Usage: find <file> <word>\n");
        return;
    }

    char *file = make_absolute_file(args[0]);
    char *word = args[1];

    FILE *file_open = fopen(file, "r");

    if (!file_open){
        printf("Cannot open: %s\n", file);    
        return;
    }

    char row[1024];
    int n_row = 1;
    char* new_row = NULL;
    
    while (fgets(row, sizeof(row), file_open)){ 
        if (strstr(row, word)){ 

            printf(BBLUE "[%d] " RESET, n_row);
            for (int i = 0; row[i] != '\0'; i++){
                if (strchr(word, row[i])){   
                    printf(BGREEN "%c" RESET, row[i]);
                } else {
                    printf("%c", row[i]);
                }
            }
        }     

        if (row[strlen(row) -1] != '\n'){
            printf("\n");
        }

        n_row++;
    }

    fclose(file_open);
    _handle_last_single_output(file);
}

void handle_read(char **args, int argc){
    if (argc != 1){  
        printf("Usage: read <file>\n");
        return;
    }

    char *file = make_absolute_file(args[0]);

    if (!file || strlen(file) == 0) {
        printf("Invalid path\n");
        free(file); 
        return;
    }

    FILE *file_open = fopen(file, "r");
    if (!file_open) {
        printf("Cannot open: %s\n", file);
        free(file);
        return;
    }

    char row[1024];
    int n_row = 1;

    while (fgets(row, sizeof(row), file_open)) {
        printf(BBLUE "[%d] " RESET "%s", n_row, row);

        if (row[strlen(row) -1] != '\n'){
            printf("\n");
        }
        n_row++;
    }

    fclose(file_open);
    _handle_last_single_output(file);
    free(file);
}

void handle_write(char **args, int argc){
    if (argc != 2){  
        printf("Usage: write <file> <text>\n");
        return;
    }

    char *file = make_absolute_file(args[0]);
    
    if (!file || strlen(file) == 0) {
        printf("Invalid path\n");
        free(file); 
        return;
    }

    long file_size = get_file_size(file);

    if (file_size > 0){ 
        char userChoice;
        printf("File already contains %ld bytes.\n", file_size);

        while (1){  
            printf("You wanna overwite ? [Y | N] ");
            scanf(" %c", &userChoice);
            if (userChoice == 'Y' || userChoice == 'y'){ 
                break;
            }
            else if (userChoice == 'N' || userChoice == 'n'){   
                return;
            }
        }
    }

    FILE *file_open = fopen(file, "w");
    if (!file_open){
        printf("Cannot open or create file: %s\n", file);    
        free(file);
        return;
    }

    fprintf(file_open,"%s\n", args[1]);
    printf("Text written to file: %s\n", file);

    fclose(file_open);
    free(file);
    _handle_last_single_output(file);
}