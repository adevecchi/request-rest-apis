#ifndef VALIDATION_H
#define VALIDATION_H

#include <stddef.h>

int is_valid_date_string(const char *date_string);
int decrement_one_day(char *date_string, size_t date_string_size);
int is_valid_zip_code(const char *zip_code);
void normalize_zip_code(const char *zip_code, char *normalized_zip_code, size_t normalized_size);

#endif // VALIDATION_H
