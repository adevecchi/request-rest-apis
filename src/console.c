#include <stdio.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "console.h"

#ifdef _WIN32
static UINT original_input_code_page;
static UINT original_output_code_page;
#endif

void set_console_charset(int code_page) {
#ifdef _WIN32
    original_input_code_page = GetConsoleCP();
    original_output_code_page = GetConsoleOutputCP();

    SetConsoleCP(code_page);
    SetConsoleOutputCP(code_page);
    setlocale(LC_ALL, "pt_BR.UTF-8");
#else
    (void)code_page;
    setlocale(LC_CTYPE, "");
#endif
}

void restore_console_charset(void) {
#ifdef _WIN32
    SetConsoleCP(original_input_code_page);
    SetConsoleOutputCP(original_output_code_page);
#endif
}
