#ifndef COMMANDS_UTILS_H
#define COMMANDS_UTILS_H

void handle_help(char **args, int argc);
void handle_unknown(char *arg);
void handle_exit(char **args, int argc);
void handle_compress(char **args, int argc);
void handle_decompress(char **args, int argc);
void handle_version(char **args, int argc);
void _handle_too_many_args(char **args,int argc);

#endif 