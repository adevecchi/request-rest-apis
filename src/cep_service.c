#include <stdio.h>
#include <string.h>
#include "api_framework.h"
#include "cep_service.h"
#include "validation.h"

static const ApiModelField address_fields[] = {
    {"cep", API_JSON_STRING, 1},
    {"logradouro", API_JSON_STRING, 0},
    {"bairro", API_JSON_STRING, 0},
    {"localidade", API_JSON_STRING, 0},
    {"uf", API_JSON_STRING, 0},
    {"estado", API_JSON_STRING, 0}
};

static const ApiResponseModel address_model = {
    "ViaCEP",
    API_JSON_OBJECT,
    NULL,
    address_fields,
    sizeof(address_fields) / sizeof(address_fields[0])
};

cJSON *cep_service_fetch_address(const char *zip_code, ApiError *error) {
    char url[128];
    char file_name[20];
    char normalized_zip_code[9];
    ApiEndpoint endpoint;

    if (zip_code == NULL) {
        return NULL;
    }

    if (!is_valid_zip_code(zip_code)) {
        if (error != NULL) {
            error->code = API_ERROR_INVALID_ARGUMENT;
            error->http_status = 0;
            snprintf(error->message, sizeof(error->message),
                     "CEP inválido: %s.", zip_code);
        }
        return NULL;
    }

    normalize_zip_code(zip_code, normalized_zip_code, sizeof(normalized_zip_code));
    snprintf(url, sizeof(url), "https://viacep.com.br/ws/%s/json", normalized_zip_code);
    snprintf(file_name, sizeof(file_name), "cache_%s.json", normalized_zip_code);

    endpoint.url = url;
    endpoint.cache_file = file_name;
    endpoint.use_cache = 1;
    endpoint.cache_ttl_seconds = 86400;
    endpoint.response_model = &address_model;

    return api_fetch_endpoint(&endpoint, error);
}
