#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "api_framework.h"
#include "cep_use_case.h"
#include "console.h"
#include "validation.h"

static void print_address(const cJSON *address);


int main(int argc, char *argv[]) {
    const char *zip_code = (argc > 1) ? argv[1] : "01001000";
    cJSON *address = NULL;
    ApiError error;

    if (argc > 2) {
        fprintf(stderr, "Uso: %s [CEP]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc > 1 && !is_valid_zip_code(zip_code)) {
        fprintf(stderr, "Erro: CEP inválido (%s). Use 8 dígitos ou o formato 00000-000.\n", zip_code);
        return EXIT_FAILURE;
    }

    set_console_charset(UTF8);

    if (!cep_use_case_get_address(zip_code, &address, &error)) {
        api_error_print(stderr, &error);
        restore_console_charset();
        return EXIT_FAILURE;
    }

    print_address(address);
    cJSON_Delete(address);
    restore_console_charset();
    return EXIT_SUCCESS;
}


static void print_address(const cJSON *address) {
    if (address == NULL) {
        return;
    }

    const cJSON *zip_code = cJSON_GetObjectItemCaseSensitive(address, "cep");
    const cJSON *street = cJSON_GetObjectItemCaseSensitive(address, "logradouro");
    const cJSON *neighborhood = cJSON_GetObjectItemCaseSensitive(address, "bairro");
    const cJSON *city = cJSON_GetObjectItemCaseSensitive(address, "localidade");
    const cJSON *state_code = cJSON_GetObjectItemCaseSensitive(address, "uf");
    const cJSON *state = cJSON_GetObjectItemCaseSensitive(address, "estado");

    printf("Endereço Encontrado:\n\n");
    if (cJSON_IsString(zip_code) && zip_code->valuestring != NULL) printf("CEP:    %s\n", zip_code->valuestring);
    if (cJSON_IsString(street) && street->valuestring != NULL) printf("Rua:    %s\n", street->valuestring);
    if (cJSON_IsString(neighborhood) && neighborhood->valuestring != NULL) printf("Bairro: %s\n", neighborhood->valuestring);
    if (cJSON_IsString(city) && city->valuestring != NULL) printf("Cidade: %s\n", city->valuestring);
    if (cJSON_IsString(state_code) && state_code->valuestring != NULL) printf("UF:     %s\n", state_code->valuestring);
    if (cJSON_IsString(state) && state->valuestring != NULL) printf("Estado: %s\n\n", state->valuestring);
}
