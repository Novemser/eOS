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
#include "stdarg.h"

int main(int argc, char const *argv[])
{

	int fd = open(argv[1], O_CREAT | O_RDWR);
	if (fd == -1)
	{
		printf("Failed to create file, Please check the file name!\n");
		return 1;
	}

	printf("File created successfully: %s (fd %d)\n", argv[1], fd);
	close(fd);

	return 0;
}