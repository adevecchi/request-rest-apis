#include <curl/curl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "request.h"

#define HTTP_CONNECT_TIMEOUT_SECONDS 10L
#define HTTP_TOTAL_TIMEOUT_SECONDS 30L

static int curl_global_initialized = 0;

static int ensure_curl_global_initialized(void) {
    if (curl_global_initialized) {
        return 1;
    }

    CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
    if (result != CURLE_OK) {
        return 0;
    }

    curl_global_initialized = 1;
    return 1;
}

static void release_curl_global_if_unused(void) {
    if (curl_global_initialized) {
        curl_global_cleanup();
        curl_global_initialized = 0;
    }
}

static size_t callback_memory_write(void *content, size_t size, size_t nmemb, void *userp) {
    if (size != 0 && nmemb > SIZE_MAX / size) {
        return 0;
    }

    size_t real_size = size * nmemb;
    struct ResponseBuffer *mem = (struct ResponseBuffer *)userp;

    if (content == NULL || userp == NULL) {
        return 0;
    }

    if (mem->size == SIZE_MAX || real_size > SIZE_MAX - mem->size - 1) {
        return 0;
    }

    char *ptr = realloc(mem->text, mem->size + real_size + 1);
    if (ptr == NULL) {
        return 0;
    }

    mem->text = ptr;
    memcpy(&(mem->text[mem->size]), content, real_size);
    mem->size += real_size;
    mem->text[mem->size] = '\0';

    return real_size;
}

void free_http_response(HttpResponse *response) {
    if (response == NULL) {
        return;
    }

    free(response->body);
    memset(response, 0, sizeof(*response));
}

static const char *http_method_name(HttpMethod method) {
    switch (method) {
        case HTTP_METHOD_GET: return "GET";
        case HTTP_METHOD_POST: return "POST";
        case HTTP_METHOD_PUT: return "PUT";
        case HTTP_METHOD_DELETE: return "DELETE";
        default: return NULL;
    }
}

int http_request_detailed(const char *url, HttpMethod method, const char *body, const char *content_type, HttpResponse *response) {
    CURL *curl_handle;
    CURLcode res;
    struct ResponseBuffer chunk = {0};
    char error_buffer[CURL_ERROR_SIZE] = {0};
    struct curl_slist *headers = NULL;
    char *content_type_header = NULL;
    const char *method_name = http_method_name(method);

    if (response == NULL) {
        return 0;
    }
    memset(response, 0, sizeof(*response));
    if (url == NULL) {
        snprintf(response->transport_message, sizeof(response->transport_message),
                 "URL não informada.");
        release_curl_global_if_unused();
        return 0;
    }
    if (method_name == NULL) {
        snprintf(response->transport_message, sizeof(response->transport_message),
                 "Método HTTP não suportado.");
        release_curl_global_if_unused();
        return 0;
    }
    if (method == HTTP_METHOD_GET && body != NULL) {
        snprintf(response->transport_message, sizeof(response->transport_message),
                 "Requisições GET não aceitam corpo.");
        release_curl_global_if_unused();
        return 0;
    }
    if (content_type != NULL) {
        size_t content_type_length = strlen(content_type);
        static const char content_type_prefix[] = "Content-Type: ";

        if (strchr(content_type, '\r') != NULL || strchr(content_type, '\n') != NULL ||
            content_type_length > SIZE_MAX - sizeof(content_type_prefix)) {
            snprintf(response->transport_message, sizeof(response->transport_message),
                     "Cabeçalho Content-Type inválido.");
            return 0;
        }

        content_type_header = malloc(sizeof(content_type_prefix) + content_type_length);
        if (content_type_header == NULL) {
            snprintf(response->transport_message, sizeof(response->transport_message),
                     "Não foi possível alocar o cabeçalho da requisição.");
            return 0;
        }
        snprintf(content_type_header, sizeof(content_type_prefix) + content_type_length,
                 "%s%s", content_type_prefix, content_type);
        headers = curl_slist_append(headers, content_type_header);
        if (headers == NULL) {
            snprintf(response->transport_message, sizeof(response->transport_message),
                     "Não foi possível alocar os cabeçalhos da requisição.");
            free(content_type_header);
            return 0;
        }
    }

    chunk.text = malloc(1);
    if (chunk.text == NULL) {
        snprintf(response->transport_message, sizeof(response->transport_message),
                 "Não foi possível alocar o buffer da resposta.");
        curl_slist_free_all(headers);
        free(content_type_header);
        return 0;
    }
    chunk.text[0] = '\0';

    if (!ensure_curl_global_initialized()) {
        snprintf(response->transport_message, sizeof(response->transport_message),
                 "Não foi possível inicializar o cliente HTTP.");
        free(chunk.text);
        curl_slist_free_all(headers);
        free(content_type_header);
        return 0;
    }

    curl_handle = curl_easy_init();
    if (curl_handle == NULL) {
        snprintf(response->transport_message, sizeof(response->transport_message),
                 "Não foi possível inicializar o cliente HTTP.");
        free(chunk.text);
        curl_slist_free_all(headers);
        free(content_type_header);
        release_curl_global_if_unused();
        return 0;
    }

    res = curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    if (res == CURLE_OK && method == HTTP_METHOD_GET) res = curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
    if (res == CURLE_OK && method != HTTP_METHOD_GET) res = curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, method_name);
    if (res == CURLE_OK && body != NULL) res = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)strlen(body));
    if (res == CURLE_OK && body != NULL) res = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, body);
    if (res == CURLE_OK && headers != NULL) res = curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    if (res == CURLE_OK) res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, callback_memory_write);
    if (res == CURLE_OK) res = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    if (res == CURLE_OK) res = curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_buffer);
    if (res == CURLE_OK) res = curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
    if (res == CURLE_OK) res = curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
    if (res == CURLE_OK) res = curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, HTTP_CONNECT_TIMEOUT_SECONDS);
    if (res == CURLE_OK) res = curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, HTTP_TOTAL_TIMEOUT_SECONDS);
    if (res == CURLE_OK) res = curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);

    if (res != CURLE_OK) {
        snprintf(response->transport_message, sizeof(response->transport_message),
                 "%s", curl_easy_strerror(res));
        free(chunk.text);
        curl_easy_cleanup(curl_handle);
        curl_slist_free_all(headers);
        free(content_type_header);
        release_curl_global_if_unused();
        return 0;
    }

    res = curl_easy_perform(curl_handle);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response->status_code);
        response->body = chunk.text;
        response->body_size = chunk.size;
    } else {
        response->transport_error = (int)res;
        snprintf(response->transport_message, sizeof(response->transport_message),
                 "%s", error_buffer[0] != '\0' ? error_buffer : curl_easy_strerror(res));
        free(chunk.text);
    }

    curl_easy_cleanup(curl_handle);
    curl_slist_free_all(headers);
    free(content_type_header);
    release_curl_global_if_unused();
    return response->body != NULL;
}

int http_get_detailed(const char *url, HttpResponse *response) {
    return http_request_detailed(url, HTTP_METHOD_GET, NULL, NULL, response);
}

char *http_request(const char *url, HttpMethod method, const char *body, const char *content_type) {
    HttpResponse response;
    if (!http_request_detailed(url, method, body, content_type, &response)) {
        return NULL;
    }

    if (response.status_code < 200 || response.status_code >= 300) {
        free_http_response(&response);
        return NULL;
    }

    char *response_body = response.body;
    response.body = NULL;
    free_http_response(&response);
    return response_body;
}

char *http_get(const char *url) {
    return http_request(url, HTTP_METHOD_GET, NULL, NULL);
}
