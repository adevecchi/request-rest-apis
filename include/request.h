#ifndef REQUEST_H
#define REQUEST_H

#include <stddef.h>

struct ResponseBuffer {
    char *text;
    size_t size;
};

typedef enum HttpMethod {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE
} HttpMethod;

typedef struct HttpResponse {
    char *body;
    size_t body_size;
    long status_code;
    int transport_error;
    char transport_message[256];
} HttpResponse;

char *http_request(const char *url, HttpMethod method, const char *body, const char *content_type);
int http_request_detailed(const char *url, HttpMethod method, const char *body, const char *content_type, HttpResponse *response);

char *http_get(const char *url);
int http_get_detailed(const char *url, HttpResponse *response);

void free_http_response(HttpResponse *response);

#endif // REQUEST_H
