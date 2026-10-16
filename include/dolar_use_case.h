#ifndef DOLAR_USE_CASE_H
#define DOLAR_USE_CASE_H

#include "api_framework.h"
#include "cJSON.h"

int dolar_use_case_get_quote(const char *requested_date, cJSON **out_response, ApiError *error);

#endif // DOLAR_USE_CASE_H
