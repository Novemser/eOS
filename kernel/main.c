
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

extern long startup_time;
extern long kernel_mktime(struct tm * tm);

// Getting time from CMOS
#define CMOS_READ(addr) ({ \
    outb_p(0x80|addr,0x70); \
    inb_p(0x71); \
})

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static void init_time(void)
{
	struct tm time;

	do {
		time.tm_sec = CMOS_READ(0);
		time.tm_min = CMOS_READ(2);
		time.tm_hour = CMOS_READ(4);
		time.tm_mday = CMOS_READ(7);
		time.tm_mon = CMOS_READ(8)-1;
		time.tm_year = CMOS_READ(9);
	} while (time.tm_sec != CMOS_READ(0));
	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	startup_time = kernel_mktime(&time);
}
/*****************************************************************************
 *                               kernel_main
 *****************************************************************************/
/**
 * jmp from kernel.asm::_start. 
 * 
 *****************************************************************************/
 PUBLIC int kernel_main()
 {
 	disp_str("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
 	init_time();
 	init_process();
 	init_clock();
 	init_keyboard();

	disp_str("Welcome!\n");
	disp_str("Kernel startup at\n");
 	
 	disp_start_timel();

 	restart();

 	while(1){}
 }

void init_process() 
{
	int i, j, eflags, prio;
	u8  rpl;
        u8  priv; /* privilege */

	struct task * t;
	struct proc * p = proc_table;

	char * stk = task_stack + STACK_SIZE_TOTAL;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++,p++,t++) {
		if (i >= NR_TASKS + NR_NATIVE_PROCS) {
			p->p_flags = FREE_SLOT;
			continue;
		}

		if (i < NR_TASKS) 
		{
        	/* TASK */
			t	= task_table + i;
			priv	= PRIVILEGE_TASK;
			rpl     = RPL_TASK;
            eflags  = 0x1202;/* IF=1, IOPL=1, bit 2 is always 1 */
			prio    = 15;
		}
		else 
		{                  
        	/* USER PROC */
			t	= user_proc_table + (i - NR_TASKS);
			priv	= PRIVILEGE_USER;
			rpl     = RPL_USER;
            eflags  = 0x202;	/* IF=1, bit 2 is always 1 */
			prio    = 5;
		}

		strcpy(p->name, t->name);	/* name of the process */
		p->pid = i;
		p->p_parent = NO_TASK;

		if (strcmp(t->name, "INIT") != 0) {
			p->ldts[INDEX_LDT_C]  = gdt[SELECTOR_KERNEL_CS >> 3];
			p->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

			/* change the DPLs */
			p->ldts[INDEX_LDT_C].attr1  = DA_C   | priv << 5;
			p->ldts[INDEX_LDT_RW].attr1 = DA_DRW | priv << 5;
		}
		else 
		{		
			/* INIT process */
			unsigned int k_base;
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_desc(&p->ldts[INDEX_LDT_C],
				  0, /* bytes before the entry point
				      * are useless (wasted) for the
				      * INIT process, doesn't matter
				      */
				      (k_base + k_limit) >> LIMIT_4K_SHIFT,
				      DA_32 | DA_LIMIT_4K | DA_C | priv << 5);

			init_desc(&p->ldts[INDEX_LDT_RW],
				  0, /* bytes before the entry point
				      * are useless (wasted) for the
				      * INIT process, doesn't matter
				      */
				      (k_base + k_limit) >> LIMIT_4K_SHIFT,
				      DA_32 | DA_LIMIT_4K | DA_DRW | priv << 5);
		}

		p->state = TASK_RUNNING;
		p->regs.cs = INDEX_LDT_C << 3 |	SA_TIL | rpl;
		p->regs.ds =
		p->regs.es =
		p->regs.fs =
		p->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;
		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p->regs.eip	= (u32)t->initial_eip;
		p->regs.esp	= (u32)stk;
		p->regs.eflags	= eflags;

		p->ticks = p->priority = prio;

		p->p_flags = 0;
		p->p_msg = 0;
		p->p_recvfrom = NO_TASK;
		p->p_sendto = NO_TASK;
		p->has_int_msg = 0;
		p->q_sending = 0;
		p->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p->filp[j] = 0;

		stk -= t->stacksize;
	}

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;
}

void disp_start_timel()
{
	unsigned char m,s,h,y,mon,d;
	s=CMOS_READ(0);//秒
	m=CMOS_READ(2);//分
	h=CMOS_READ(4);//时
	y=CMOS_READ(9);
	mon=CMOS_READ(8)-1;
	d=CMOS_READ(7);

	disp_str("20");
	disp_int(BCD_TO_BIN(y));
	disp_str("/");
	disp_int(BCD_TO_BIN(mon));
	disp_str("/");
	disp_int(BCD_TO_BIN(d));
	disp_str(" ");
	disp_int(BCD_TO_BIN(h));
	disp_str(":");
	disp_int(BCD_TO_BIN(m));
	disp_str(":");
	disp_int(BCD_TO_BIN(s));
	disp_str("\n");
}

struct time get_time_RTC()
{
	struct time t;
	MESSAGE msg;
	msg.type = GET_RTC_TIME;
	msg.BUF= &t;
	send_recv(BOTH, TASK_SYS, &msg);
	return t;
}
/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}

void show_proc_info()
{
	int total = NR_TASKS + NR_NATIVE_PROCS + forked_proc_cnt;
	int running = 0;
    printf("\n=============================================================================\n");

    printf("|   PID   |   Process Name   |  Running Ticks  |    State   |\n");
    
    printf("-----------------------------------------------------------------------------\n");

    // Iterate all the processes exist in the OS
    for (int i = 0 ; i < NR_TASKS + NR_NATIVE_PROCS + forked_proc_cnt; ++i )
    {
    	// Minor display improvement....23333
    	if (proc_table[i].pid >= 10)
    	{
    		printf("  %d          %s             %d             ", proc_table[i].pid, proc_table[i].name, proc_table[i].priority);
    		
    	}
    	else
    	{
    		printf("  %d           %s               %d              ", proc_table[i].pid, proc_table[i].name, proc_table[i].priority);

    	}
    	// Show different process state
    	switch(proc_table[i].state)
    	{
    		case TASK_READY:
	    		printf("READY\n");
	    		break;
    		case TASK_RUNNING:
	    		printf("RUNNING\n");
				running++;
	    		break;
    		case TASK_BLOCKED:
	    		printf("BLOCKED\n");
	    		break;
 			default:
 				assert(1>2);
 				break;
    	}
    }

    printf("=============================================================================\n");
    printf("Total process:     %d\n", total);
    printf("Running:           %d\n\n", running);
}

/**
 * @struct posix_tar_header
 * Borrowed from GNU `tar'
 */
 struct posix_tar_header
{				/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
	/* 500 */
};

/*****************************************************************************
 *                                untar
 *****************************************************************************/
/**
 * Extract the tar file and store them.
 * 
 * @param filename The tar file.
 *****************************************************************************/
void untar(const char * filename)
{
	//printf("[extract `%s'\n", filename);
	int fd = open(filename, O_RDWR);
	assert(fd != -1);
	int cnt2 = 0;

	char buf[SECTOR_SIZE * 16];
	int chunk = sizeof(buf);

	while (1) {
		read(fd, buf, SECTOR_SIZE);
		if (buf[0] == 0)
			break;

		cnt2++;

		struct posix_tar_header * phdr = (struct posix_tar_header *)buf;

	/* calculate the file size */
		char * p = phdr->size;
		int f_len = 0;
		while (*p)
		f_len = (f_len * 8) + (*p++ - '0'); /* octal */

			int bytes_left = f_len;
		int fdout = open(phdr->name, O_CREAT | O_RDWR);
		if (fdout == -1) {
			printf("Failed to extract file: %s\n", phdr->name);
			printf("Extracting aborted");
			return;
		}
		//printf("    %s (%d bytes)\n", phdr->name, f_len);
		while (bytes_left) {
			int iobytes = min(chunk, bytes_left);
			read(fd, buf,
				((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			write(fdout, buf, iobytes);
			bytes_left -= iobytes;
		}
		close(fdout);
	}

	close(fd);

	printf("Extract %d binary files done.\n", cnt2);
}

/*****************************************************************************
 *                                shabby_shell
 *****************************************************************************/
/**
 * A very very simple shell.
 * 
 * @param tty_name  TTY file name.
 *****************************************************************************/
void shabby_shell(const char * tty_name)
{
	int fd_stdin  = open(tty_name, O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[128];

	shell_routine();

	close(1);
	close(0);
}

void shell_routine()
{
	char rdbuf[128];
	
	while (1) 
	{
		write(1, "novemser$ ", 10);
		int r = read(0, rdbuf, 70);
		rdbuf[r] = 0;

		int argc = 0;
		char * argv[PROC_ORIGIN_STACK];
		char * p = rdbuf;
		char * s;
		int word = 0;
		char ch;
		do {
			ch = *p;
			if (*p != ' ' && *p != 0 && !word) 
			{
				s = p;
				word = 1;
			}
			if ((*p == ' ' || *p == 0) && word) 
			{
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while(ch);
		argv[argc] = 0;

		int fd = open(argv[0], O_RDWR);
		if (fd == -1) 
		{
			if (rdbuf[0]) 
			{
				// Recognize some command
				if (strcmp(rdbuf, "showproc") == 0)
				{
					show_proc_info();
				}
				else if (strcmp(rdbuf, "time") == 0)
				{
					struct time t = get_time_RTC();
					printf("%d/%d/%d %d:%d:%d\n", t.year, t.month, t.day, t.hour, t.minute, t.second);
				}
				else
				{
					printf("Unrecognized command ");
					write(1, "\"", 1);
					write(1, rdbuf, r);
					write(1, "\"\n", 2);
				}
			}
		}
		else 
		{
			close(fd);
			int pid = fork();
			if (pid != 0) 
			{ /* parent */
				int s;
				wait(&s);
			}
			else 
			{	/* child */
				execv(argv[0], argv);
			}
		}
	}
}
/*****************************************************************************
 *                                Init
 *****************************************************************************/
/**
 * The hen.
 * 
 *****************************************************************************/
void Init()
{
	int fd_stdin  = open("/dev_tty0", O_RDWR);
	assert(fd_stdin  == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	printf("Now loading...\n");
	/* extract `cmd.tar' */
	untar("/cmd.tar");
	printf("\nLoading finished. Use \"help\" to know more.\n");


	char * tty_list[] = {"/dev_tty1", "/dev_tty2"};

	int i;
	for (i = 0; i < sizeof(tty_list) / sizeof(tty_list[0]); i++) 
	{
		int pid = fork();
		/* parent process */
		if (pid != 0) {}
		/* child process */
		else
		{
			close(fd_stdin);
			close(fd_stdout);

			shabby_shell(tty_list[i]);
			assert(0);
		}
	}

	milli_delay(1000);
	printf("\n");
	shell_routine();
	
	while (1) {
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s);
	}

	assert(0);
}


/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	for(;;);
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	for(;;);
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC()
{
	for(;;);
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
 PUBLIC void panic(const char *fmt, ...)
 {
 	int i;
 	char buf[256];

	/* 4 is the size of fmt in the stack */
 	va_list arg = (va_list)((char*)&fmt + 4);

 	i = vsprintf(buf, fmt, arg);

 	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
 	__asm__ __volatile__("ud2");
 }

// Clear the screen
void clear()
{
	clear_screen(0, console_table[current_console].cursor);
	console_table[current_console].crtc_start = 0;
	console_table[current_console].cursor = 0;
}