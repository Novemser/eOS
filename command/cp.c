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
    // Get the content of the file
	int fd = open(argv[2], O_RDWR);
	char buf[1024];

	if (fd == -1)
	{
		printf("File does not exist, please check the file name!\n");
		return 1;
	}

	int tail = read(fd, buf, 1024);
	close(fd);

    // Create a new file
	fd = open(argv[1], O_CREAT | O_RDWR);
	// If the file already exists
	if (fd == -1)
	{  // Do nothing
	}
	else
	{
        // File does not exist
		char temp[1024];
		temp[0] = 0;
		write(fd, temp, 1);
		close(fd);
	}
    //给文件赋值
	fd = open(argv[1], O_RDWR);
	write(fd, buf, tail+1);
	close(fd);
	return 0;
}