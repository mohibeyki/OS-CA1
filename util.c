
#include "util.h"
#include <unistd.h>
#include <stdarg.h>

char* readLine() {
	char* buf = (char*) malloc(sizeof(char));
	char* str = (char*) malloc(sizeof(char));
	strcpy(str, "");
	read(0, buf, 1);
	while (*buf != 13 && *buf != 10) {
		strncat(str, buf, 1);
		read(0, buf, 1);
	}
	return str;
}

int writeLine(char* tmp) {
	if (write(1, tmp, strlen(tmp)) < 0) {
		write(2, "Error\n", 6);
		return -1;
	}
	return 0;
}

int writeMany(int n_args, ...) {
	register int i;
	va_list ap;
	va_start(ap, n_args);
	int flag = 0;
	for (i = 0; i < 1; i++) {
		printf("%d\n", i);
		char* tmp = va_arg(ap, char*);
		if (write(1, tmp, strlen(tmp)) < 0) {
			write(2, "Error\n", 6);
			flag = -1;
		}
	}
	va_end(ap);

	return flag;
}

int writeErr(char* str) {
	if (write(2, str, strlen(str)) < 0) {
		write(1, "Error\n", 6);
		return -1;
	}
	return 0;
}

int indexOf(char* src, char c) {
	int len = strlen(src);
	int i;
	for (i = 0; i < len; i++) {
		if (src[i] == c)
			return i;
	}
	return -1;
}
