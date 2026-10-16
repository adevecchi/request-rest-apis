#ifndef DOLAR_SERVICE_H
#define DOLAR_SERVICE_H

#include "api_framework.h"
#include "cJSON.h"

cJSON *dolar_service_fetch_quote(const char *requested_date, ApiError *error);

#endif // DOLAR_SERVICE_H
