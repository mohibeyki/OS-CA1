#include"DataObjects.h"

void resetClient( Client* cli)
{
	(*cli).fileCount=0;
	(*cli).name[0] = 0;
	(*cli).id = -1;
	(*cli).fd = -1;
}
