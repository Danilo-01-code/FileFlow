#ifndef UTILS_H
#define UTILS_H

void ensure_config_dir_exists(const char *path);
char **split(char *word);
int count_words(char *sentence);

#endif 