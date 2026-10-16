# Request REST APIs

Coleção de exemplos em C para consumo de APIs REST usando `libcurl` e `cJSON`.

## Objetivo

Demonstrar padrões práticos de:
- requisição HTTP
- validação de entrada
- parsing e validação de JSON
- cache local
- organização modular em projetos pequenos em C

## Arquitetura

O projeto foi organizado em camadas bem definidas:

- `validation.*`: validação reutilizável de CEP e data
- `request.*`: cliente HTTP genérico
- `http_json.*`: leitura e parse de JSON bruto
- `cache.*`: cache local com TTL e validação do nome do arquivo
- `api_framework.*`: camada de integração com APIs, validação de modelos e tratamento de erros
- `*_service.*`: integração com serviços externos específicos
- `*_use_case.*`: regras de negócio e validação de inputs de cada fluxo
- executáveis em `src/*.c`: camadas de entrada/CLI

## Mini-framework

A camada `api_framework` centraliza os padrões recorrentes para consumo de APIs:

- `api_fetch_raw()` busca o payload bruto
- `api_fetch_json()` busca e parseia o JSON em um passo
- `api_fetch_json_cached()` reaproveita cache e TTL configuráveis
- `api_fetch_endpoint()` combina cache, parsing, validação de modelo e padronização de erros

Cada endpoint pode definir um `ApiResponseModel` com:
- tipo raiz JSON
- campo de coleção opcional
- campos esperados e obrigatórios

Se a resposta for um JSON válido, mas incompatível com o modelo esperado, a operação será rejeitada como erro de modelo.

### Tratamento de erros

`ApiError` encapsula a categoria do erro e uma mensagem padronizada para:
- entrada inválida
- transporte HTTP
- status HTTP fora da faixa 2xx
- JSON inválido
- incompatibilidade com o modelo

Use `api_error_print(stderr, &error)` para exibir mensagens de erro de forma consistente.

## Estrutura do projeto

- `src/viacep.c`: CLI para consulta por CEP
- `src/cotacaodolar.c`: CLI para consulta de cotação do dólar
- `src/jsonplaceholder.c`: exemplo de consumo de API genérica
- `src/request.c`: cliente HTTP genérico
- `src/http_json.c`: leitura e parse de JSON
- `src/api_framework.c`: framework de integração
- `src/cache.c`: cache local
- `src/validation.c`: validação reutilizável
- `src/cep_service.c`: serviço externo para ViaCEP
- `src/dolar_service.c`: serviço externo para PTAX do Banco Central
- `src/cep_use_case.c`: caso de uso para CEP
- `src/dolar_use_case.c`: caso de uso para cotação do dólar
- `include/`: headers públicos

## Build

No Windows, com GCC/MinGW e libcurl instalados, use:

```powershell
mingw32-make -f Makefile clean all
```

No Linux, com GCC, GNU Make e libcurl instalados, use:

```sh
make clean all
```

O Makefile usa `pkg-config` para localizar as flags de compilação e link do `libcurl` quando disponível; caso contrário, usa `-lcurl`.

## Executáveis gerados

- `build/viacep` (Linux) / `build/viacep.exe` (Windows)
- `build/jsonplaceholder` (Linux) / `build/jsonplaceholder.exe` (Windows)
- `build/cotacaodolar` (Linux) / `build/cotacaodolar.exe` (Windows)

## Exemplos de uso

### CEP

```bash
./build/viacep 01001000
```

### Cotação do dólar

```bash
./build/cotacaodolar 04-10-2026
```

## Observações

- o projeto foi estruturado para fins de estudo e demonstração
- o cache é usado para reduzir requisições repetidas
- entradas inválidas são rejeitadas antes da chamada HTTP
- o código foi organizado para facilitar extensão e reutilização
