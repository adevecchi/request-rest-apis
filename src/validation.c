#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "validation.h"

int is_valid_date_string(const char *date_string) {
    int day, month, year;
    struct tm parsed_date = {0};

    if (date_string == NULL || sscanf(date_string, "%d-%d-%d", &day, &month, &year) != 3) {
        return 0;
    }

    if (day < 1 || month < 1 || month > 12 || year < 1900 || year > 9999) {
        return 0;
    }

    parsed_date.tm_mday = day;
    parsed_date.tm_mon = month - 1;
    parsed_date.tm_year = year - 1900;
    parsed_date.tm_hour = 12;
    parsed_date.tm_isdst = -1;

    if (mktime(&parsed_date) == (time_t)-1) {
        return 0;
    }

    return parsed_date.tm_mday == day &&
           parsed_date.tm_mon == month - 1 &&
           parsed_date.tm_year == year - 1900;
}

int decrement_one_day(char *date_string, size_t date_string_size) {
    int day, month, year;
    struct tm parsed_date = {0};

    if (date_string == NULL || date_string_size == 0 || !is_valid_date_string(date_string)) {
        return 0;
    }

    if (sscanf(date_string, "%d-%d-%d", &day, &month, &year) != 3) {
        return 0;
    }

    parsed_date.tm_mday = day;
    parsed_date.tm_mon = month - 1;
    parsed_date.tm_year = year - 1900;
    parsed_date.tm_hour = 12;
    parsed_date.tm_isdst = -1;

    if (mktime(&parsed_date) == (time_t)-1) {
        return 0;
    }

    parsed_date.tm_mday -= 1;
    if (mktime(&parsed_date) == (time_t)-1) {
        return 0;
    }

    snprintf(date_string, date_string_size, "%02d-%02d-%04d",
             parsed_date.tm_mday, parsed_date.tm_mon + 1, parsed_date.tm_year + 1900);
    return 1;
}

int is_valid_zip_code(const char *zip_code) {
    size_t length;
    size_t digits = 0;

    if (zip_code == NULL) {
        return 0;
    }

    length = strlen(zip_code);
    if (length != 8 && length != 9) {
        return 0;
    }

    for (size_t index = 0; index < length; index++) {
        unsigned char current = (unsigned char)zip_code[index];
        if (current == '-') {
            if (index != 5) {
                return 0;
            }
            continue;
        }

        if (current < '0' || current > '9') {
            return 0;
        }

        digits++;
    }

    return digits == 8;
}

void normalize_zip_code(const char *zip_code, char *normalized_zip_code, size_t normalized_size) {
    size_t normalized_index = 0;

    if (normalized_zip_code == NULL || normalized_size == 0) {
        return;
    }
    normalized_zip_code[0] = '\0';

    if (zip_code == NULL) {
        return;
    }

    for (size_t index = 0; zip_code[index] != '\0'; index++) {
        if (zip_code[index] != '-') {
            if (normalized_index + 1 >= normalized_size) {
                normalized_zip_code[0] = '\0';
                return;
            }
            normalized_zip_code[normalized_index++] = zip_code[index];
        }
    }

    normalized_zip_code[normalized_index] = '\0';
}
