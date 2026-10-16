#ifndef API_FRAMEWORK_H
#define API_FRAMEWORK_H

#include <stddef.h>
#include <stdio.h>
#include "cJSON.h"

typedef enum ApiJsonType {
    API_JSON_OBJECT,
    API_JSON_ARRAY,
    API_JSON_STRING,
    API_JSON_NUMBER,
    API_JSON_BOOL
} ApiJsonType;

typedef struct ApiModelField {
    const char *name;
    ApiJsonType type;
    int required;
} ApiModelField;

typedef struct ApiResponseModel {
    const char *name;
    ApiJsonType root_type;
    const char *records_field;
    const ApiModelField *fields;
    size_t field_count;
} ApiResponseModel;

typedef struct ApiEndpoint {
    const char *url;
    const char *cache_file;
    int use_cache;
    int cache_ttl_seconds;
    const ApiResponseModel *response_model;
} ApiEndpoint;

typedef enum ApiErrorCode {
    API_ERROR_NONE,
    API_ERROR_INVALID_ARGUMENT,
    API_ERROR_TRANSPORT,
    API_ERROR_HTTP,
    API_ERROR_JSON,
    API_ERROR_MODEL
} ApiErrorCode;

typedef struct ApiError {
    ApiErrorCode code;
    long http_status;
    char message[256];
} ApiError;

char *api_fetch_raw(const char *url);
char *api_fetch_raw_checked(const char *url, ApiError *error);
cJSON *api_fetch_json(const char *url);
cJSON *api_fetch_json_cached(const ApiEndpoint *endpoint);
cJSON *api_fetch_endpoint(const ApiEndpoint *endpoint, ApiError *error);
void api_error_print(FILE *stream, const ApiError *error);

#endif // API_FRAMEWORK_H
