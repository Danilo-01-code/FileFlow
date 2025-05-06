#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minizip/zip.h>

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
    printf("  dcmp <input_path> <output_path> <new_file_name>  Decompresses the ZIP file at <input_path> to <output_path>\n\n");
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
        printf("Usage: cmp <input_path> <output_path> <compress_file_name>\n");
        return;
    }

    const char *in_file = args[0];
    char *out_dir = args[1];
    char *out_name;
    int needsFree = 0;

    if (argc == 3){  
        out_name = ensure_zip_extension(args[2]);
        needsFree = 1;
    }
    else{   
        out_name = "out.zip";
    }

    char zip_path[1024];
    snprintf(zip_path, sizeof(zip_path), "%s/%s", out_dir, out_name);

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
    if(needsFree){  
        free(out_name);
    }
}

void handle_decompress(char **args, int argc){  
    //TODO
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