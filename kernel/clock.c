/*************************************************************************//**
 *****************************************************************************
 * @file   clock.c
 * @brief  
 * @author Novemser
 * @date   2016
 *****************************************************************************
 *****************************************************************************/

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

extern int default_proc_load;

/*****************************************************************************
 *                                clock_handler
 *****************************************************************************/
/**
 * <Ring 0> This routine handles the clock interrupt generated by 8253/8254
 *          programmable interval timer.
 *
 *			It will do the task scheduling job using CFS algorithm.
 * 
 * @param irq The IRQ nr, unused here.
 *****************************************************************************/
PUBLIC void clock_handler(int irq)
{
	// These global ticks are very important signals!!
	// Touch at your own risk!!
	if (++ticks >= MAX_TICKS)
		ticks = 0;

	// Tong shang
	if (key_pressed)
		inform_int(TASK_TTY);

	if (k_reenter != 0) {
		return;
	}

	int total = NR_TASKS + NR_NATIVE_PROCS + forked_proc_cnt;
	int sum_runtime;
	int delta = p_proc_ready->se.vruntime;
	int cfs_total_weight = 0;


	// Calculate the total weight of all processes
	for (int i = 0; i < total; ++i)
	{
		cfs_total_weight += proc_table[i].se.weight;
	}

	// Set sum runtime according to the total number of processes
	if (total <= 5)
		sum_runtime = sysctl_sched_latency;
	else
		sum_runtime = sysctl_sched_min_granularity * total;
	
	// Set current ideal runtime
	p_proc_ready->se.ideal_time = sum_runtime * p_proc_ready->se.weight / cfs_total_weight;
	// Recalculate the new virtual runtime
	p_proc_ready->se.vruntime += default_proc_load / p_proc_ready->se.weight;

	// Current process' runtime exceeds its ideal runtime
	// Trigger schedule
	if (p_proc_ready->se.vruntime > p_proc_ready->se.ideal_time)
	{
		// Running out of time, change state to ready
		p_proc_ready->state = TASK_READY;
		schedule();
	}


}

/*****************************************************************************
 *                                milli_delay
 *****************************************************************************/
/**
 * <Ring 1~3> Delay for a specified amount of time.
 * 
 * @param milli_sec How many milliseconds to delay.
 *****************************************************************************/
PUBLIC void milli_delay(int milli_sec)
{
    int t = get_ticks();

    while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*****************************************************************************
 *                                init_clock
 *****************************************************************************/
/**
 * <Ring 0> Initialize 8253/8254 PIT (Programmable Interval Timer).
 * 
 *****************************************************************************/
PUBLIC void init_clock()
{
    /* 初始化 8253 PIT */
    out_byte(TIMER_MODE, RATE_GENERATOR);
    out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
    out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

    put_irq_handler(CLOCK_IRQ, clock_handler);    /* 设定时钟中断处理程序 */
    enable_irq(CLOCK_IRQ);                        /* 让8259A可以接收时钟中断 */
}


