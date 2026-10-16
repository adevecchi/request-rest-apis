#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "api_framework.h"

static const ApiModelField user_fields[] = {
    {"id", API_JSON_NUMBER, 1},
    {"name", API_JSON_STRING, 1},
    {"email", API_JSON_STRING, 1},
    {"address", API_JSON_OBJECT, 1}
};

static const ApiResponseModel users_model = {
    "JSONPlaceholder users",
    API_JSON_ARRAY,
    NULL,
    user_fields,
    sizeof(user_fields) / sizeof(user_fields[0])
};

static void print_user(const cJSON *user);


int main(void) {
    const ApiEndpoint endpoint = {
        "https://jsonplaceholder.typicode.com/users",
        NULL,
        0,
        0,
        &users_model
    };
    ApiError error;
    cJSON *users = api_fetch_endpoint(&endpoint, &error);
    if (users == NULL) {
        api_error_print(stderr, &error);
        return 1;
    }

    const int total_users = cJSON_GetArraySize(users);
    printf("Total de usuarios: %d\n\n", total_users);

    for (int index = 0; index < total_users; index++) {
        cJSON *user = cJSON_GetArrayItem(users, index);
        print_user(user);
    }

    cJSON_Delete(users);
    return 0;
}


static void print_user(const cJSON *user) {
    if (user == NULL) {
        return;
    }

    const cJSON *id = cJSON_GetObjectItemCaseSensitive(user, "id");
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(user, "name");
    const cJSON *email = cJSON_GetObjectItemCaseSensitive(user, "email");
    const cJSON *address = cJSON_GetObjectItemCaseSensitive(user, "address");

    if (id != NULL && cJSON_IsNumber(id)) {
        printf("Usuario #%d:\n", id->valueint);
    }

    if (cJSON_IsString(name) && name->valuestring != NULL) {
        printf("  Nome:  %s\n", name->valuestring);
    }

    if (cJSON_IsString(email) && email->valuestring != NULL) {
        printf("  Email: %s\n", email->valuestring);
    }

    if (address != NULL && cJSON_IsObject(address)) {
        const cJSON *city = cJSON_GetObjectItemCaseSensitive(address, "city");
        const cJSON *street = cJSON_GetObjectItemCaseSensitive(address, "street");

        if (cJSON_IsString(city) && city->valuestring != NULL &&
            cJSON_IsString(street) && street->valuestring != NULL) {
            printf("  End.:  %s, %s\n", street->valuestring, city->valuestring);
        }
    }

    printf("\n");
}
