CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

COMMON_SRCS = src/request.c src/http_json.c src/api_framework.c src/cache.c src/validation.c src/cep_service.c src/dolar_service.c src/cep_use_case.c src/dolar_use_case.c src/cJSON.c

ifeq ($(OS),Windows_NT)
    EXE_EXT = .exe
    MKDIR = if not exist build mkdir build
    RM = if exist build rmdir /s /q build
    CURL_CFLAGS ?=
    CURL_LIBS ?= -lcurl
else
    EXE_EXT =
    MKDIR = mkdir -p build
    RM = rm -rf build
    CURL_CFLAGS ?= $(shell pkg-config --cflags libcurl 2>/dev/null)
    CURL_LIBS ?= $(shell pkg-config --libs libcurl 2>/dev/null || echo -lcurl)
endif

TARGETS = build/viacep$(EXE_EXT) build/jsonplaceholder$(EXE_EXT) build/cotacaodolar$(EXE_EXT)

all: build_dir $(TARGETS)

build_dir:
	@$(MKDIR)

build/viacep$(EXE_EXT): src/viacep.c src/console.c $(COMMON_SRCS)
	$(CC) $(CFLAGS) $(CURL_CFLAGS) src/viacep.c src/console.c $(COMMON_SRCS) -o $@ $(CURL_LIBS)

build/jsonplaceholder$(EXE_EXT): src/jsonplaceholder.c $(COMMON_SRCS)
	$(CC) $(CFLAGS) $(CURL_CFLAGS) src/jsonplaceholder.c $(COMMON_SRCS) -o $@ $(CURL_LIBS)

build/cotacaodolar$(EXE_EXT): src/cotacaodolar.c src/console.c $(COMMON_SRCS)
	$(CC) $(CFLAGS) $(CURL_CFLAGS) src/cotacaodolar.c src/console.c $(COMMON_SRCS) -o $@ $(CURL_LIBS)

.PHONY: all clean build_dir

clean:
	@$(RM)
