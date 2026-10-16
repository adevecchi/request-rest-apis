#include <stdio.h>
#include <string.h>
#include "api_framework.h"
#include "cep_service.h"
#include "cep_use_case.h"
#include "validation.h"

int cep_use_case_get_address(const char *zip_code, cJSON **out_address, ApiError *error) {
    if (out_address == NULL) {
        if (error != NULL) {
            error->code = API_ERROR_INVALID_ARGUMENT;
            error->http_status = 0;
            snprintf(error->message, sizeof(error->message),
                     "Erro: o buffer de endereço não foi informado.");
        }
        return 0;
    }

    *out_address = NULL;

    if (zip_code == NULL) {
        if (error != NULL) {
            error->code = API_ERROR_INVALID_ARGUMENT;
            error->http_status = 0;
            snprintf(error->message, sizeof(error->message),
                     "Erro: o CEP não foi informado.");
        }
        return 0;
    }

    if (!is_valid_zip_code(zip_code)) {
        if (error != NULL) {
            error->code = API_ERROR_INVALID_ARGUMENT;
            error->http_status = 0;
            snprintf(error->message, sizeof(error->message),
                     "Erro: CEP inválido (%s). Use 8 dígitos ou o formato 00000-000.", zip_code);
        }
        return 0;
    }

    *out_address = cep_service_fetch_address(zip_code, error);
    return *out_address != NULL;
}
