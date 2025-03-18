#define _POSIX_C_SOURCE 200809L 
#define _XOPEN_SOURCE 700       
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/types.h>
#include <limits.h>

typedef struct {
    ino_t ino;
    char *name;
} FileEntry;

static int calculate_ino_width(ino_t ino) {
    return snprintf(NULL, 0, "%" PRIuMAX, (uintmax_t)ino);
}

static bool should_skip_entry(const struct dirent *entry) {
    return (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0] == '.');
}

void process_directory(const char *dirname) {
    if (!dirname)
        return;
    DIR *dir = opendir(dirname);
    if (!dir)
        return;

    FileEntry *files = NULL;
    size_t count = 0;
    size_t max_ino_width = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (should_skip_entry(entry)) {
            continue;
        }

        char full_path[PATH_MAX];
        int ret = snprintf(full_path, sizeof(full_path), "%s/%s", dirname, entry->d_name);
        if (ret < 0) {
            fprintf(stderr, "Invalid input\n");
            continue;
        }
        if (ret >= (int)sizeof(full_path)) {
            fprintf(stderr, "Invalid input\n");
            continue;
        }

        struct stat statbuf;
        if (lstat(full_path, &statbuf) == -1) {
            fprintf(stderr, "Invalid input\n");
            continue;
        }

        char *name_copy = strdup(entry->d_name);
        if (name_copy == NULL) {
            fprintf(stderr, "Invalid input\n");
            for (size_t j = 0; j < count; j++) free(files[j].name);
            free(files);
            closedir(dir);
            exit(EXIT_FAILURE);
        }

        FileEntry fe = { .ino = statbuf.st_ino, .name = name_copy };

        FileEntry *temp = realloc(files, (count + 1) * sizeof(FileEntry));
        if (temp == NULL) {
            fprintf(stderr, "Invalid input\n");
            free(name_copy);
            for (size_t j = 0; j < count; j++) free(files[j].name);
            free(files);
            closedir(dir);
            exit(EXIT_FAILURE);
        }
        files = temp;
        files[count++] = fe;

        int width = calculate_ino_width(fe.ino);
        if (width > (int)max_ino_width) max_ino_width = width;
    }

    closedir(dir);

    for (size_t i = 0; i < count; i++) {
        printf("%*" PRIuMAX " %s\n", (int)max_ino_width, (uintmax_t)files[i].ino, files[i].name);
    }

    for (size_t i = 0; i < count; i++) free(files[i].name);
    free(files);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        process_directory(".");
    } else {
        for (int i = 1; i < argc; i++) process_directory(argv[i]);
    }
    return 0;
}