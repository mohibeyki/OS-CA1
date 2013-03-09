
#ifndef __UTILITIES_H_
#define __UTILITIES_H_

#include <unistd.h>
#include <stdarg.h>
#include <math.h>

int writeMany(int n_args, ...);
int writeLine(char* tmp);
int writeErr(char* str);
char* readLine();
int indexOf(char* src, char c);
char* itoa(int i);

#endif
