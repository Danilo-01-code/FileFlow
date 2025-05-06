#ifndef UTILS_H
#define UTILS_H

void ensure_config_dir_exists(const char *path);
char **split(char *word, size_t wordLen);
size_t count_words(char *sentence);
int count_args(char **args);
int dir_exists(const char *path);
int create_dir(const char *path);

#endif 