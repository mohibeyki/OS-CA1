#include "FileUtil.h"
#include "../Consts/Consts.h"

int nextToken(char* str, char* ans, int index) {
	memset(ans, 0, BUFFER_SIZE);
	for(; str[index] && str[index] != 10 && str[index] != ' ' && str[index] != '\t' && str[index] != 13; index++)
		strncat(ans, &(str[index]), 1);
	return index + 1;
}

int nextTokenDelimiter(char* str, char* output, int index, char d) {
	strcpy(output, "");
	for(; str[index] && str[index] != d; index++) {
		strncat(output, &(str[index]), 1);
	}
	return index + 1;
}

int indexOf(char* src, char c) {
	int len = strlen(src);
	int i = 0;
	for (; i < len; i++) {
		if (src[i] == c)
			return i;
	}
	return -1;
}
