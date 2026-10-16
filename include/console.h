#ifndef CONSOLE_H
#define CONSOLE_H

#define UTF8 65001

void set_console_charset(int code_page);
void restore_console_charset(void);

#endif
