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
	int result;
	result = unlink(argv[1]);
	if (result == 0)
	{
		printf("File deleted successfully!\n");
		return 0;
	}
	else
	{
		printf("Failed to delete file, please check your file name!\n");
		return 1;
	}
}