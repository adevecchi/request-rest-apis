#ifndef HTTP_JSON_H
#define HTTP_JSON_H

#include <stddef.h>
#include "cJSON.h"

char *fetch_json_text(const char *url);
cJSON *parse_json_document(const char *json_text);
cJSON *fetch_json_document(const char *url);

#endif // HTTP_JSON_H
