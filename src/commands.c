#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    printf(GREEN "These are some FileFlow commands:\n\n" RESET);
    printf("  cmp <input_path> <output_path> <new_file_name>   Compresses the file at <input_path> and saves on <output_path>\n");
    printf("  dcmp <input_path> <output_path>                  Decompresses the ZIP file at <input_path> to <output_path>\n\n");
    printf("  version             See the current Version\n");
    printf("  clear               Clear the REPL\n");
    printf("  curr                see the current directory\n");
    printf("  arc                 See your computer architecture (64 or 32 bits)\n");
    printf("  name                Your current OS name\n");
    printf("  exit                Exit the system\n\n");
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
    if (argc != 2 && argc != 3) {
        printf("Usage: cmp <input_path> <output_path> <new_file_name>\n");
        return;
    }

    const char *in_file = args[0];
    char *out_dir = args[1];
    char *out_file;
    int needs_free = 0;

    if (argc == 3){  
        out_file = ensure_zip_extension(args[2]);
        needs_free = 1;
    }
    else{   
        out_file = "out.zip";
    }

    char zip_path[1024];
    snprintf(zip_path, sizeof(zip_path), "%s/%s", out_dir, out_file);

    if (!dir_exists(out_dir)) {       
        if (strcmp(out_dir, ".") == 0){    
            out_dir = _get_cwd();
        }
        else{
            printf(YELLOW "Directory do not found. trying to create: %s\n" RESET, out_dir);

            if (create_dir(out_dir) != 0) {
                return;
            }
            printf(GREEN "Directory created successfully!\n" RESET);
        }
    }
    
    zipFile zf = zipOpen(zip_path, APPEND_STATUS_CREATE);
    if (!zf) {
        fprintf(stderr, RED "Cannot create the zip File: %s\n" RESET, zip_path);
        return;
    }

    FILE *fp = fopen(in_file, "rb");
    if (!fp) {
        fprintf(stderr, RED "Cannot open the File on the input:\n" RESET "%s\n", in_file);
        zipClose(zf, NULL);
        return;
    }

    const char *filename = strrchr(in_file, '/');
    if (!filename) filename = in_file;
    else filename++;

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

    printf(GREEN "Zip file created @:" RESET "%s\n", zip_path);
    if(needs_free){  
        free(out_file);
    }
}

void handle_decompress(char **args, int argc){  
    if (argc != 2) {
        printf("Usage: cmp <input_path> <output_path>\n");
        return;
    }

    char* in_file = args[0];
    char* out_dir = args[1];

    unzFile zip_input = unzOpen(in_file);
    if (!zip_input) {
        printf(RED "Cannot open the zip file.\n" RESET);
        free(in_file);
        return;
    }
    
    do {
        char filename[256];
        unz_file_info info;
        if (unzGetCurrentFileInfo(zip_input, &info, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK) {
            break;
        }

        size_t fn_len = strlen(filename);
        if (fn_len > 0 && filename[fn_len - 1] == '/') {
            char dir_path[512];
            snprintf(dir_path, sizeof(dir_path), "%s/%s", out_dir, filename);
            create_dir(dir_path);
            continue;
        }

        // Create parent dir
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", out_dir, filename);

        if (strcmp(out_dir, ".") == 0) {
            out_dir = _get_cwd();
        }

        char parent[512];
        strcpy(parent, full_path);
        char *p = strrchr(parent, '/');
        if (p) {
            *p = '\0';
            create_dir(parent);
        }

        // Extract
        if (unzOpenCurrentFile(zip_input) != UNZ_OK) {
            printf(RED "Failed to open entry: %s\n" RESET, filename);
            break;
        }

        FILE *out = fopen(full_path, "wb");
        if (!out) {
            printf(RED "Failed to create file: %s\n" RESET, full_path);
            unzCloseCurrentFile(zip_input);
            break;
        }

        char buffer[8192];
        int bytes;
        while ((bytes = unzReadCurrentFile(zip_input, buffer, sizeof(buffer))) > 0) {
            fwrite(buffer, 1, bytes, out);
        }

        fclose(out);
        unzCloseCurrentFile(zip_input);

        printf(GREEN "Extracted @: %s\n" RESET, full_path);

    } while (unzGoToNextFile(zip_input) == UNZ_OK);

    unzClose(zip_input);
}

void handle_version(char **args, int argc){ 
    if (argc != 0){ 
        _handle_too_many_args(args,argc);
        return;
    }
    printf("%f\n",VERSION);
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
    char * curr = _get_cwd();
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