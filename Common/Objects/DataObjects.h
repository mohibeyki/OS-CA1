
#ifndef __DATAOBJECTS_H_
#define __DATAOBJECTS_H_

#include"../Consts/Consts.h"

typedef struct {
	char name[BUFFER_SIZE];
	int id;
	int fd;
	char fileNames[FILE_SIZE][BUFFER_SIZE];
	int fileCount;
} Client;

void resetClient( Client* cli);


#endif
