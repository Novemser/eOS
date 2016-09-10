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
	char rdbuf[1024];
	int fd = open(argv[1], O_RDWR);

    if (fd == -1)
    {
        printf("File does not exist, please check the file name!\n");
        return 1;
    }

    int tail = read(0, rdbuf, 1024);
    rdbuf[tail] = 0;

    write(fd, rdbuf, tail + 1);
    close(fd);
    
	return 0;
}