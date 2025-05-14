#ifndef UTILS_H
#define UTILS_H

extern char *prompt;
extern char *userInput;

void ensure_config_dir_exists(const char *path);
char *ensure_zip_extension(const char *filename);
char **split(char *word, size_t wordLen);
size_t count_words(char *sentence);
int count_args(char **args);
int dir_exists(const char *path);
int create_dir(const char *path);
void remove_dir(const char *path);
char *make_absolute_dir(const char* path);
char *make_absolute_file(const char* path);
int is_subdir(const char *parent, const char *child);
long long get_directory_size(const char* dir_path);
long get_file_size(const char* file);
void cleanup_and_exit(char **prompt, char **userInput, int exit_code);
void to_lowercase(char *str);
void remove_dir(const char* path);

#endif 