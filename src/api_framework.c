#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "api_framework.h"
#include "http_json.h"
#include "cache.h"
#include "request.h"

static void set_api_error(ApiError *error, ApiErrorCode code, long status, const char *message) {
    if (error == NULL) {
        return;
    }

    error->code = code;
    error->http_status = status;
    snprintf(error->message, sizeof(error->message), "%s", message);
}

static const char *http_status_description(long status) {
    switch (status) {
        case 400: return "requisição inválida";
        case 401: return "não autorizado";
        case 403: return "acesso proibido";
        case 404: return "recurso não encontrado";
        case 408: return "tempo de requisição esgotado";
        case 409: return "conflito";
        case 429: return "limite de requisições excedido";
        case 500: return "erro interno do servidor";
        case 502: return "gateway inválido";
        case 503: return "serviço indisponível";
        case 504: return "tempo de resposta do gateway esgotado";
        default:
            if (status >= 400 && status < 500) return "erro na requisição";
            if (status >= 500 && status < 600) return "erro no servidor";
            return "resposta HTTP inesperada";
    }
}

static int is_json_type_compatible(const cJSON *value, ApiJsonType type) {
    switch (type) {
        case API_JSON_OBJECT: return cJSON_IsObject(value);
        case API_JSON_ARRAY: return cJSON_IsArray(value);
        case API_JSON_STRING: return cJSON_IsString(value);
        case API_JSON_NUMBER: return cJSON_IsNumber(value);
        case API_JSON_BOOL: return cJSON_IsBool(value);
        default: return 0;
    }
}

static int validate_response_model(const cJSON *document, const ApiResponseModel *model, ApiError *error) {
    if (model == NULL) {
        return 1;
    }

    if (model->name == NULL ||
        model->root_type < API_JSON_OBJECT || model->root_type > API_JSON_BOOL ||
        (model->field_count > 0 && model->fields == NULL) ||
        (model->records_field != NULL && model->root_type != API_JSON_OBJECT)) {
        set_api_error(error, API_ERROR_INVALID_ARGUMENT, 0,
                     "Erro: a definição do modelo de resposta é inválida.");
        return 0;
    }

    for (size_t field_index = 0; field_index < model->field_count; field_index++) {
        if (model->fields[field_index].name == NULL ||
            model->fields[field_index].type < API_JSON_OBJECT ||
            model->fields[field_index].type > API_JSON_BOOL) {
            set_api_error(error, API_ERROR_INVALID_ARGUMENT, 0,
                             "Erro: a definição do modelo contém um campo inválido.");
            return 0;
        }
    }

    if (!is_json_type_compatible(document, model->root_type)) {
        char message[256];
        snprintf(message, sizeof(message),
                 "Erro: resposta incompatível com o modelo '%s': tipo raiz inesperado.",
                 model->name);
        set_api_error(error, API_ERROR_MODEL, 0, message);
        return 0;
    }

    const cJSON *records = document;
    if (model->records_field != NULL) {
        records = cJSON_GetObjectItemCaseSensitive(document, model->records_field);
        if (!cJSON_IsArray(records)) {
            char message[256];
            snprintf(message, sizeof(message),
                     "Erro: resposta incompatível com o modelo '%s': campo '%s' ausente ou não é uma lista.",
                     model->name, model->records_field);
            set_api_error(error, API_ERROR_MODEL, 0, message);
            return 0;
        }
    }

    int count = cJSON_IsArray(records) ? cJSON_GetArraySize(records) : 1;
    for (int index = 0; index < count; index++) {
        const cJSON *record = cJSON_IsArray(records)
            ? cJSON_GetArrayItem(records, index)
            : records;
        if (!cJSON_IsObject(record)) {
            set_api_error(error, API_ERROR_MODEL, 0,
                         "Erro: a resposta contém um registro que não é um objeto JSON.");
            return 0;
        }

        for (size_t field_index = 0; field_index < model->field_count; field_index++) {
            const ApiModelField *field = &model->fields[field_index];
            const cJSON *value = cJSON_GetObjectItemCaseSensitive(record, field->name);
            if ((value == NULL && field->required) ||
                (value != NULL && !is_json_type_compatible(value, field->type))) {
                char message[256];
                snprintf(message, sizeof(message),
                         "Erro: resposta incompatível com o modelo '%s': campo '%s' ausente ou inválido.",
                         model->name, field->name);
                set_api_error(error, API_ERROR_MODEL, 0, message);
                return 0;
            }
        }
    }

    return 1;
}

char *api_fetch_raw_checked(const char *url, ApiError *error) {
    HttpResponse response;

    if (error != NULL) {
        memset(error, 0, sizeof(*error));
    }
    if (url == NULL) {
        set_api_error(error, API_ERROR_INVALID_ARGUMENT, 0,
                     "A URL da requisição não foi informada.");
        return NULL;
    }

    if (!http_get_detailed(url, &response)) {
        char message[256];
        snprintf(message, sizeof(message), "Falha de transporte HTTP: %.*s",
                 (int)(sizeof(message) - sizeof("Falha de transporte HTTP: ")),
                 response.transport_message[0] != '\0'
                     ? response.transport_message : "erro desconhecido");
        set_api_error(error, API_ERROR_TRANSPORT, 0, message);
        free_http_response(&response);
        return NULL;
    }
    if (response.status_code < 200 || response.status_code >= 300) {
        char message[256];
        snprintf(message, sizeof(message), "HTTP %ld: %s.",
                 response.status_code, http_status_description(response.status_code));
        set_api_error(error, API_ERROR_HTTP, response.status_code, message);
        free_http_response(&response);
        return NULL;
    }

    char *body = response.body;
    response.body = NULL;
    free_http_response(&response);
    return body;
}

char *api_fetch_raw(const char *url) {
    return api_fetch_raw_checked(url, NULL);
}

cJSON *api_fetch_json(const char *url) {
    ApiEndpoint endpoint = {url, NULL, 0, 0, NULL};
    return api_fetch_endpoint(&endpoint, NULL);
}

cJSON *api_fetch_json_cached(const ApiEndpoint *endpoint) {
    return api_fetch_endpoint(endpoint, NULL);
}

cJSON *api_fetch_endpoint(const ApiEndpoint *endpoint, ApiError *error) {
    char *json_text = NULL;
    cJSON *document = NULL;

    if (error != NULL) {
        memset(error, 0, sizeof(*error));
    }
    if (endpoint == NULL || endpoint->url == NULL) {
        set_api_error(error, API_ERROR_INVALID_ARGUMENT, 0,
                     "Erro: o endpoint ou sua URL não foi informado.");
        return NULL;
    }

    if (endpoint->use_cache && endpoint->cache_file != NULL) {
        int ttl = endpoint->cache_ttl_seconds > 0 ? endpoint->cache_ttl_seconds : 86400;
        if (is_cache_valid_for_ttl(endpoint->cache_file, ttl)) {
            json_text = read_cache_file(endpoint->cache_file);
            if (json_text != NULL) {
                document = parse_json_document(json_text);
                free(json_text);
                json_text = NULL;

                if (document != NULL &&
                    validate_response_model(document, endpoint->response_model, error)) {
                    return document;
                }

                cJSON_Delete(document);
                document = NULL;
                if (error != NULL) {
                    memset(error, 0, sizeof(*error));
                }
            }
        }
    }

    json_text = api_fetch_raw_checked(endpoint->url, error);
    if (json_text == NULL) {
        return NULL;
    }

    document = parse_json_document(json_text);
    if (document == NULL) {
        free(json_text);
        set_api_error(error, API_ERROR_JSON, 0,
                     "Erro: a resposta do endpoint não contém um JSON válido.");
        return NULL;
    }
    if (!validate_response_model(document, endpoint->response_model, error)) {
        free(json_text);
        cJSON_Delete(document);
        return NULL;
    }

    if (endpoint->use_cache && endpoint->cache_file != NULL) {
        write_cache_file(endpoint->cache_file, json_text);
    }
    free(json_text);

    return document;
}

void api_error_print(FILE *stream, const ApiError *error) {
    if (stream == NULL || error == NULL || error->code == API_ERROR_NONE) {
        return;
    }
    fprintf(stream, "[API] %s\n", error->message);
}
