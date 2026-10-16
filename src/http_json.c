#include <stdlib.h>
#include "http_json.h"
#include "request.h"

char *fetch_json_text(const char *url) {
    if (url == NULL) {
        return NULL;
    }

    return http_get(url);
}

cJSON *parse_json_document(const char *json_text) {
    if (json_text == NULL) {
        return NULL;
    }

    return cJSON_Parse(json_text);
}

cJSON *fetch_json_document(const char *url) {
    char *json_text = fetch_json_text(url);
    if (json_text == NULL) {
        return NULL;
    }

    cJSON *document = parse_json_document(json_text);
    free(json_text);
    return document;
}
