#ifndef CACHE_H
#define CACHE_H

int is_cache_valid(const char *file_name);
int is_cache_valid_for_ttl(const char *file_name, int ttl_seconds);
char *read_cache_file(const char *file_name);
void write_cache_file(const char *file_name, const char *json_text);

#endif // CACHE_H
