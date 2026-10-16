#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "cache.h"

#define CACHE_EXPIRATION_SECONDS 86400
#define CACHE_FILE_NAME_MAX_LENGTH 255

static int is_valid_cache_file_name(const char *file_name) {
    size_t length;

    if (file_name == NULL || *file_name == '\0') {
        return 0;
    }

    length = strlen(file_name);
    if (length == 0 || length >= CACHE_FILE_NAME_MAX_LENGTH) {
        return 0;
    }

    if (strchr(file_name, '/') != NULL || strchr(file_name, '\\') != NULL) {
        return 0;
    }

    if (strstr(file_name, "..") != NULL) {
        return 0;
    }

    return 1;
}

int is_cache_valid_for_ttl(const char *file_name, int ttl_seconds) {
    struct stat file_status;

    if (!is_valid_cache_file_name(file_name) || ttl_seconds < 0 || stat(file_name, &file_status) != 0) {
        return 0;
    }

    time_t current_time = time(NULL);
    time_t last_modified = file_status.st_mtime;
    double elapsed_seconds = difftime(current_time, last_modified);

    if (elapsed_seconds > ttl_seconds) {
        return 0;
    }

    return 1;
}

int is_cache_valid(const char *file_name) {
    return is_cache_valid_for_ttl(file_name, CACHE_EXPIRATION_SECONDS);
}

char *read_cache_file(const char *file_name) {
    FILE *file;
    long file_size;
    char *buffer;
    size_t bytes_read;

    if (!is_valid_cache_file_name(file_name)) {
        return NULL;
    }

    file = fopen(file_name, "r");
    if (file == NULL) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    buffer = malloc((size_t)file_size + 1U);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    bytes_read = fread(buffer, 1, (size_t)file_size, file);
    if (bytes_read != (size_t)file_size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

void write_cache_file(const char *file_name, const char *json_text) {
    FILE *file;

    if (!is_valid_cache_file_name(file_name) || json_text == NULL) {
        return;
    }

    file = fopen(file_name, "w");
    if (file == NULL) {
        return;
    }

    if (fprintf(file, "%s", json_text) < 0) {
        fclose(file);
        return;
    }

    fclose(file);
}
