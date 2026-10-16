#include <stdio.h>
#include <string.h>
#include "api_framework.h"
#include "dolar_service.h"
#include "dolar_use_case.h"
#include "validation.h"

int dolar_use_case_get_quote(const char *requested_date, cJSON **out_response, ApiError *error) {
    if (out_response == NULL) {
        if (error != NULL) {
            error->code = API_ERROR_INVALID_ARGUMENT;
            error->http_status = 0;
            snprintf(error->message, sizeof(error->message),
                     "Erro: o buffer da cotação não foi informado.");
        }
        return 0;
    }

    *out_response = NULL;

    if (requested_date == NULL) {
        if (error != NULL) {
            error->code = API_ERROR_INVALID_ARGUMENT;
            error->http_status = 0;
            snprintf(error->message, sizeof(error->message),
                     "Erro: a data da cotação não foi informada.");
        }
        return 0;
    }

    if (!is_valid_date_string(requested_date)) {
        if (error != NULL) {
            error->code = API_ERROR_INVALID_ARGUMENT;
            error->http_status = 0;
            snprintf(error->message, sizeof(error->message),
                     "Erro: data inválida (%s). Use o formato DD-MM-AAAA.", requested_date);
        }
        return 0;
    }

    *out_response = dolar_service_fetch_quote(requested_date, error);
    return *out_response != NULL;
}
