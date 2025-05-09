#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <minizip/zip.h>
#include <minizip/unzip.h>

#include "../includes/defs.h"
#include "hostNameUtils.h"
#include "utils.h"

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
        "  cmp  <input_path> <output_path>          Compresses the file at <input_path> and saves on <output_path>\n"
        "  dcmp <input_path> <output_path>          Decompresses the ZIP file at <input_path> to <output_path>\n"
        "  mv   <input_path> <output_path> -c | -x  Copy (-c flag) or Cut (-x flag) the files of a given <input_file> to an <output_file>\n"
        "       -c : Copy mode (the original files remain in place)\n"
        "       -x : Cut mode (the original files are removed after transfer)\n\n"
        "  mk   <dir | file>   Make a directory or a file. Write '/example' for create a dir and 'example.extension' for a file\n"
        "  rm   <dir | file>   Remove a file or a directory\n"
        "  version             See the current Version\n"
        "  clear               Clear the REPL\n"
        "  curr                See the current directory\n"
        "  arc                 See your computer architecture (64 or 32 bits)\n"
        "  name                Your current OS name\n"
        "  exit                Exit the system\n"
        "  welcome             Change the welcome config\n"
        "  cd                  Change directory\n\n");
}

void handle_unknown(char *arg){
    printf("%s:" RED " command not found" RESET ", digit help to see the commands\n", arg);
}

void handle_exit(char **args, int argc){
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }
    printf("Bye Bye\n");
    exit(0);
}

void handle_compress(char **args, int argc){    
    if (argc != 2) {
        printf("Usage: cmp <input_path> <output_path> \n");
        return;
    }

    const char *in_file  = make_absolute_file(args[0]);
    const char *out_dir = make_absolute_dir(args[1]);

    if (!in_file || !out_dir) {
        fprintf(stderr, RED "Failed to resolve absolute paths\n" RESET);
        return;
    }

    const char *filename = strrchr(in_file, '/');
    filename = filename ? filename + 1 : in_file;

    // Create the .zip: "name.txt" → "name.txt.zip"
    char zip_name[512];
    snprintf(zip_name, sizeof(zip_name), "%s.zip", filename);

    char zip_path[ABS_PATH_MAX];
    snprintf(zip_path, sizeof(zip_path), "%s/%s", out_dir, zip_name);

    zipFile zf = zipOpen(zip_path, APPEND_STATUS_CREATE);
    if (!zf) {
        fprintf(stderr, RED "Cannot create zip file: %s\n" RESET, zip_path);
        return;
    }

    FILE *fp = fopen(in_file, "rb");
    if (!fp) {
        fprintf(stderr, RED "Cannot open input file: %s\n" RESET, in_file);
        zipClose(zf, NULL);
        return;
    }

    zip_fileinfo zi = {0};
    zipOpenNewFileInZip(zf, filename, &zi,
                        NULL, 0, NULL, 0, NULL,
                        Z_DEFLATED, Z_DEFAULT_COMPRESSION);

    char buffer[4096];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        zipWriteInFileInZip(zf, buffer, bytes_read);
    }

    fclose(fp);
    zipCloseFileInZip(zf);
    zipClose(zf, NULL);
    printf(GREEN "Zip file created From: " RESET "%s" GREEN " to: " RESET "%s\n",in_file, out_dir);
}

void handle_decompress(char **args, int argc){  
    if (argc != 2) {
        printf("Usage: dcmp <input_path> <output_path>\n");
        return;
    }

    const char *in_zip  = make_absolute_file(args[0]);
    char       *out_dir = make_absolute_dir(args[1]);

    unzFile zip_input = unzOpen(in_zip);
    if (!zip_input) {
        fprintf(stderr, RED "Cannot open zip file: %s\n" RESET "Verify if your put the '.zip' on the input\n", in_zip);
        return;
    }

    do {
        char filename[512];
        unz_file_info info;

        if (unzGetCurrentFileInfo(zip_input, &info,
                                 filename, sizeof(filename),
                                 NULL, 0, NULL, 0) != UNZ_OK) {
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

    } while (unzGoToNextFile(zip_input) == UNZ_OK);

    unzClose(zip_input);
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
            printf("Moved: %s -> %s\n", in_file, out_file);
        else fprintf(stderr, RED"Failed to remove original\n" RESET);
    } else {
        printf(GREEN "Copied: " RESET "%s" GREEN "->" RESET "%s\n", in_file, out_file);
    }
}

void _handle_move_dir(const char *in_dir, const char *out_dir, int move_flag) {
    char *abs_in = make_absolute_dir(in_dir);
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
    if (argc < 2 || argc > 3) {
        printf("Usage: mv <input_dir> <output_dir> [-c|-x]\n");
        return;
    }
    int move_flag = 1; 
    if (argc == 3) {
        if (!strcmp(args[2], "-x")) move_flag = 0;
        else if (!strcmp(args[2], "-c")) move_flag = 1;
        else { fprintf(stderr, "Unknown flag %s\n", args[2]); return; }
    }
    _handle_move_dir(args[0], args[1], move_flag);
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
}

void handle_make(char **args, int argc){ 
    if (argc != 1){     
        printf("Usage: mk <dir | file>\n");
        return;
    }

    char *new_file = args[0];
    if (new_file[0] == '/'){ 
        char *dir = strdup(new_file);
        if (dir == NULL) {
            printf("strdup\n");
            return;
        }

        memmove(dir, dir + 1, strlen(dir));

        if (MKDIR(dir) != 0) {
            printf("Cannot create the directory: '%s'\n", dir);
            return;
        }
        printf("directory: '/%s' created\n", dir); 
        free(dir);
        return;
    }

    FILE *f = fopen(new_file,"w");
    if (f == NULL) {
        printf("cannot create the file: '%s'\n",new_file);
        return;
    }
    fclose(f);
    printf("File: '%s' created\n", new_file);
}   

void handle_remove(char **args, int argc){  
    if (argc != 1){     
        printf("Usage: rm <dir | file>\n");
        return;
    }
}