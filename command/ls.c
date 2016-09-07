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
#include "keyboard.h"
#include "proto.h"

int main(int argc, char * argv[])
{
    MESSAGE msg;
    msg.type = LS;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}