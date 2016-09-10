#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "io.h"
#include "system.h"
#include "time.h"


int main(int argc, char const *argv[])
{
	char buf[1024];
	
	int fd = open(argv[1], O_RDWR);
	if (fd == -1)
	{
		printf("Failed to open file, please check your file name!\n");
		return 1;
	}
	read(fd, buf, 1024);
	close(fd);
	printf("%s\n", buf);
	return 0;
}