#ifndef CEP_SERVICE_H
#define CEP_SERVICE_H

#include "api_framework.h"
#include "cJSON.h"

cJSON *cep_service_fetch_address(const char *zip_code, ApiError *error);

#endif // CEP_SERVICE_H
