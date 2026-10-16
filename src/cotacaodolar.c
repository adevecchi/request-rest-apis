#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "api_framework.h"
#include "console.h"
#include "dolar_use_case.h"
#include "validation.h"

static void print_quote(const cJSON *quotes, const char *requested_date);


int main(int argc, char *argv[]) {
    char requested_date[12];
    cJSON *response = NULL;
    cJSON *quotes = NULL;
    int attempt = 1;
    const int max_attempts = 5;
    int exit_code = EXIT_FAILURE;
    ApiError error;

    if (argc > 2) {
        fprintf(stderr, "Uso: %s [DD-MM-AAAA]\n", argv[0]);
        return EXIT_FAILURE;
    }

    snprintf(requested_date, sizeof(requested_date), "%s", (argc > 1) ? argv[1] : "04-10-2026");

    if (argc > 1 && !is_valid_date_string(requested_date)) {
        fprintf(stderr, "Erro: data inválida (%s). Use o formato DD-MM-AAAA.\n", requested_date);
        return EXIT_FAILURE;
    }

    set_console_charset(UTF8);

    while (attempt <= max_attempts) {
        if (!dolar_use_case_get_quote(requested_date, &response, &error)) {
            api_error_print(stderr, &error);
            goto cleanup;
        }

        quotes = cJSON_GetObjectItemCaseSensitive(response, "value");

        if (cJSON_IsArray(quotes) && cJSON_GetArraySize(quotes) == 0) {
            printf("[Info] Sem boletim oficial para %s.\n", requested_date);
            cJSON_Delete(response);
            response = NULL;

            if (!decrement_one_day(requested_date, sizeof(requested_date))) {
                printf("[Erro] Não foi possível recalcular a data anterior.\n");
                goto cleanup;
            }

            attempt++;
            printf("[Info] Recuando consulta para: %s\n\n", requested_date);
            continue;
        }

        break;
    }

    if (attempt > max_attempts) {
        printf("[Erro Abort] Não foi possivél localizar um boletim recente nas últimas tentativas.\n");
        goto cleanup;
    }

    if (!cJSON_IsArray(quotes)) {
        printf("[cJSON Erro] Estrutura de resposta inesperada do servidor.\n");
        goto cleanup;
    }

    print_quote(quotes, requested_date);
    exit_code = EXIT_SUCCESS;

cleanup:
    cJSON_Delete(response);
    restore_console_charset();
    return exit_code;
}

static void print_quote(const cJSON *quotes, const char *requested_date) {
    cJSON *quote = cJSON_GetArrayItem((cJSON *)quotes, 0);
    if (quote == NULL || !cJSON_IsObject(quote)) {
        printf("[cJSON Erro] O boletim localizado nao contem os campos esperados.\n");
        return;
    }

    cJSON *purchase_rate = cJSON_GetObjectItemCaseSensitive(quote, "cotacaoCompra");
    cJSON *sale_rate = cJSON_GetObjectItemCaseSensitive(quote, "cotacaoVenda");
    cJSON *quote_timestamp = cJSON_GetObjectItemCaseSensitive(quote, "dataHoraCotacao");

    printf("Boletim oficial cotação dolar para %s\n\n", requested_date);
    if (cJSON_IsNumber(purchase_rate)) printf("Dolar Compra: R$ %.4f\n", purchase_rate->valuedouble);
    if (cJSON_IsNumber(sale_rate)) printf("Dolar Venda:  R$ %.4f\n", sale_rate->valuedouble);
    if (cJSON_IsString(quote_timestamp)) printf("Data / Hora:  %s\n\n", quote_timestamp->valuestring);
}
