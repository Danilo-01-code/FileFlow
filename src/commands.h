#ifndef COMMANDS_UTILS_H
#define COMMANDS_UTILS_H

void handle_help(char **args, int argc);
void handle_unknown(char *arg);
void handle_exit(char **args, int argc);
void handle_compress(char **args, int argc);
void handle_decompress(char **args, int argc);
void handle_version(char **args, int argc);
void handle_clear(char **args, int argc);
void handle_curr_directory(char **args, int argc);
void _handle_too_many_args(char **args,int argc);
void handle_architecture(char **args,int argc);
void handle_name(char **args, int argc);
void handle_move(char **args, int argc);
void _handle_move_dir(const char *in_file, const char *out_file);
void _handle_copy(char *in_dir, char *out_dir);

#endif 