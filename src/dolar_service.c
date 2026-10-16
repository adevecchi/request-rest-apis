#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include "api_framework.h"
#include "dolar_service.h"

static const ApiModelField quote_fields[] = {
    {"cotacaoCompra", API_JSON_NUMBER, 1},
    {"cotacaoVenda", API_JSON_NUMBER, 1},
    {"dataHoraCotacao", API_JSON_STRING, 1}
};

static const ApiResponseModel quote_model = {
    "BCB PTAX",
    API_JSON_OBJECT,
    "value",
    quote_fields,
    sizeof(quote_fields) / sizeof(quote_fields[0])
};

cJSON *dolar_service_fetch_quote(const char *requested_date, ApiError *error) {
    char url[256];
    char *escaped_date = curl_easy_escape(NULL, requested_date, 0);

    if (escaped_date == NULL) {
        if (error != NULL) {
            error->code = API_ERROR_INVALID_ARGUMENT;
            error->http_status = 0;
            snprintf(error->message, sizeof(error->message),
                     "Não foi possível codificar a data da cotação.");
        }
        return NULL;
    }

    snprintf(url, sizeof(url),
             "https://olinda.bcb.gov.br/olinda/servico/PTAX/versao/v1/odata/CotacaoDolarDia(dataCotacao=@dataCotacao)?%%40dataCotacao='%s'&%%24format=json",
             escaped_date);
    curl_free(escaped_date);

    const ApiEndpoint endpoint = {
        url,
        NULL,
        0,
        0,
        &quote_model
    };

    return api_fetch_endpoint(&endpoint, error);
}
