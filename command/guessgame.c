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


int isdigit(char c)
{
    return 
    (c=='1' ||
        c=='2' ||    c=='3' ||
        c=='4' ||    c=='5' ||
        c=='6' ||    c=='7' ||
        c=='8' ||    c=='9' ||
        c=='0' );
}

int my_atoi(const char *c)
{
    long long int value = 0;
    int sign = 1;
    if( *c == '+' || *c == '-' )
    {
        if( *c == '-' ) sign = -1;
        c++;
    }
    while (isdigit(*c))
    {
        value *= 10;
        value += (int) (*c-'0');
        c++;
    }
    return (value * sign);
}

int main(void) 
{

    int random_num = 0;
    int guessed_num = 0;
    int counter = 0;
    char rdbuf[128];

    struct time t;
    MESSAGE msg;
    msg.type = GET_RTC_TIME;
    msg.BUF= &t;
    send_recv(BOTH, TASK_SYS, &msg);

    random_num = t.minute*t.second % 49 + 1;

    printf("Guess my number! (Tip:Less than 50)\n"); 

    while(1)
    {
        counter++;

        int r = read(0, rdbuf, 70);
        rdbuf[r] = 0;
        guessed_num = my_atoi(rdbuf);

        if (guessed_num == random_num) 
        {
            printf("You guessed correctly in %d tries! Congratulations!\n", counter); 
            break;
        }

        if (guessed_num < random_num) 
            printf("Your guess is too LOW. Guess again. ^_^\n");

        if (guessed_num > random_num) 
            printf("Your guess is too HIGH. Guess again. ^_^\n");

    }

    return 0;   
}