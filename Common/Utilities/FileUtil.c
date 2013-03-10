
#include "FileUtil.h"
#include "../Consts/Consts.h"

int getROFile(char* fileName) {
	return open(fileName, O_RDONLY);
}

int getWRFile(char* fileName) {
	int k = creat(fileName, S_IRWXU | O_CREAT);
	if( k >= 0)
		return k;
	return open(fileName,O_WRONLY);
}

int fReadLine(int fd, char* input) {
	char buf = 0;
	int size = 0;
	strcpy(input, "");

	int rc = read(fd, &buf, 1);
	if (rc < 0)
		return rc;

	while (buf != 13 && buf != 10) {
		input[size++] = buf;
		rc = read(fd, &buf, 1);
		if (rc < 0)
			return rc;
	}
	input[size] = 0;
	return size == strlen(input) ? size : -1;
}

int fWriteStr(int fd, char* tmp) {
	if (write(fd, tmp, strlen(tmp)) < 0) {
		write(2, "Error\n", 6);
		return -1;
	}
	return 0;
}

int fWriteChar(int fd, char c) {
	char tmp[2];
	tmp[0] = c;
	tmp[1] = 0;
	if (fWriteStr(fd, tmp))
		return -1;
	return 0;
}

int fWriteInt(int fd, int d) {
	char tmp[12];
	int i = 0;
	if (d == 0)
		return fWriteChar(fd, '0');
	if (d < 0) {
		d *= -1;
		if (fWriteChar(fd, '-') < 0)
			return -1;
	}
	while (d > 0) {
		tmp[i] = (d % 10) + '0';
		i++;
		d /= 10;
	}
	tmp[i] = 0;
	i--;
	int j = 0;
	for (; j * 2 < i; j++) {
		char c = tmp[j];
		tmp[j] = tmp[i - j];
		tmp[i - j] = c;
	}
	if (fWriteStr(fd, tmp))
		return -1;
	return 0;
}

int fprint(int fd, char* first, ...) {
	va_list vl;
	va_start(vl, first);
	char* ch = first;
	for (; *ch; ch++) {
		if (*ch != '%' && *ch != '\\') {
			if (fWriteChar(fd, *ch) < 0)
				return -1;
		} else if (*ch == '%') {
			ch++;
			if (*ch == 0)
				return 0;
			if (*ch == 's' || *ch == 'S') {
				char* tmp = va_arg(vl,char*);
				if (fWriteStr(fd, tmp) < 0)
					return -1;
			} else if (*ch == 'd') {

				int d = va_arg(vl,int);
				if (fWriteInt(fd, d) < 0)
					return -1;
			}
		} else {
			ch++;
			if (*ch == 'n') {
				if (fWriteChar(fd, '\n') < 0)
					return -1;
			} else if (*ch == 't') {
				if (fWriteChar(fd, '\t') < 0)
					return -1;
			}
		}
	}
	va_end(vl);
	return 0;
}

int fReadWord(int fd, char* output) {
	char buf = 0;
	int size = 0;
	strcpy(output, "");

	int rc = read(fd, &buf, 1);
	if (rc < 0)
		return rc;

	while (buf != 13 && buf != 10 && buf && buf != '\t' && buf != ' ') {
		output[size++] = buf;
		rc = read(fd, &buf, 1);
		if (rc < 0)
			return rc;
	}
	output[size] = 0;
	return size == strlen(output) ? size : -1;
}

int fReadInt(int fd) {
	char buf[BUFFER_SIZE];
	fReadWord(fd, buf);
	return atoi(buf);
}

int fReadSome(int fd, char* str, int count) {
	char buf;
	int size = 0;
	strcpy(str, "");
	if (read(fd, &buf, 1) != 1)
		return -1;
	str[size++] = buf;
	while (size < count && fEndOfFile(fd) == 0) {
		if (read(fd, &buf, 1) != 1)
			break;
		str[size++] = buf;
	}
	str[size] = 0;
	return size <= count ? size : -1;
}

int fscan(int fd, char* first, ...) {
	va_list vl;
	va_start(vl, first);
	char* ch = first;
	for (; *ch; ch++) {
		if (*ch == '%') {
			ch++;
			if (*ch == 0)
				return 0;
			if (*ch == 's' || *ch == 'S') {
				char buf[BUFFER_SIZE];
				fReadWord(fd, buf);
				*va_arg(vl,char**) = buf;
			} else if (*ch == 'd') {
				*va_arg(vl,int*) = fReadInt(fd);
			}
		}
	}
	va_end(vl);
	return 0;
}

int fEndOfFile(int fd) {
	char buf;
	if (read(fd, &buf, 1) != 1)
		return 1;
	lseek(fd, -1, SEEK_CUR);
	return 0;
}

