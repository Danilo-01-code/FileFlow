#include "hostNameUtils.h"

int get_computer_name(char *buffer, size_t size) {
#ifdef _WIN32
    DWORD dwSize = (DWORD)size;
    return GetComputerNameA(buffer, &dwSize) ? 0 : -1;
#else
    return gethostname(buffer, size);
#endif
}

void get_config_path(char *buffer, size_t size) {
#ifdef _WIN32
    const char *base = getenv("APPDATA");
#else
    const char *base = getenv("HOME");
#endif
    if (base == NULL) {
        fprintf(stderr, RED "Error:" RESET "Enviroment Variabe" BLUE "HOME/APPDATA" RESET "is not defined.\n");
        exit(1);
    }

    snprintf(buffer, size, "%s%cFileFlow%cconfig.txt", base, PATH_SEPARATOR, PATH_SEPARATOR);
}