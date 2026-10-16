#ifndef CEP_USE_CASE_H
#define CEP_USE_CASE_H

#include "api_framework.h"
#include "cJSON.h"

int cep_use_case_get_address(const char *zip_code, cJSON **out_address, ApiError *error);

#endif // CEP_USE_CASE_H
