/* Compiled by DJGPP */

#include <stdio.h>
#include <sys/time.h>
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


typedef long long int64_t;
#define MISSING_USLEEP

typedef struct cpuid_regs {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
} cpuid_regs_t;

static cpuid_regs_t
cpuid(int func) {
    cpuid_regs_t regs;
#define	CPUID	".byte 0x0f, 0xa2; "
    asm("movl %4,%%eax; " CPUID
        "movl %%eax,%0; movl %%ebx,%1; movl %%ecx,%2; movl %%edx,%3"
    : "=m" (regs.eax), "=m" (regs.ebx), "=m" (regs.ecx), "=m" (regs.edx)
                : "g" (func)
                : "%eax", "%ebx", "%ecx", "%edx");
    return regs;
}

static int64_t rdtsc(void)
{
    unsigned int i, j;
#define	RDTSC	".byte 0x0f, 0x31; "
asm(RDTSC : "=a"(i), "=d"(j) : );
    return ((int64_t)j<<32) + (int64_t)i;
}

static void store32(char *d, unsigned int v)
{
    d[0] =  v        & 0xff;
    d[1] = (v >>  8) & 0xff;
    d[2] = (v >> 16) & 0xff;
    d[3] = (v >> 24) & 0xff;
}

int main(int argc, char **argv)
{
    cpuid_regs_t regs, regs_ext;
    char idstr[13];
    unsigned max_cpuid;
    unsigned max_ext_cpuid;
    unsigned int amd_flags;
    char *model_name = "Unknown CPU";
    int i;
    char processor_name[49];

    regs = cpuid(0);
    max_cpuid = regs.eax;
    /* printf("%d CPUID function codes\n", max_cpuid+1); */

    store32(idstr+0, regs.ebx);
    store32(idstr+4, regs.edx);
    store32(idstr+8, regs.ecx);
    idstr[12] = 0;
    printf("vendor_id    : %s\n", idstr);

    if (strcmp(idstr, "GenuineIntel") == 0)
        model_name = "Unknown Intel CPU";
    else if (strcmp(idstr, "AuthenticAMD") == 0)
        model_name = "Unknown AMD CPU";

    regs_ext = cpuid((1<<31) + 0);
    max_ext_cpuid = regs_ext.eax;
    if (max_ext_cpuid >= (1<<31) + 1) {
        regs_ext = cpuid((1<<31) + 1);
        amd_flags = regs_ext.edx;

        if (max_ext_cpuid >= (1<<31) + 4) {
            for (i = 2; i <= 4; i++) {
                regs_ext = cpuid((1<<31) + i);
                store32(processor_name + (i-2)*16, regs_ext.eax);
                store32(processor_name + (i-2)*16 + 4, regs_ext.ebx);
                store32(processor_name + (i-2)*16 + 8, regs_ext.ecx);
                store32(processor_name + (i-2)*16 + 12, regs_ext.edx);
            }
            processor_name[48] = 0;
            model_name = processor_name;
        }
    } else {
        amd_flags = 0;
    }

    if (max_cpuid >= 1) {
        static struct {
            int bit;
            char *desc;;
            char *description;
        } cap[] = {
                      { 0,  "fpu",   "Floating-point unit on-chip" },
                      { 1,  "vme",   "Virtual Mode Enhancements" },
                      { 2,  "de",    "Debugging Extension" },
                      { 3,  "pse",   "Page Size Extension" },
                      { 4,  "tsc",   "Time Stamp Counter" },
                      { 5,  "msr",   "Pentium Processor MSR" },
                      { 6,  "pae",   "Physical Address Extension" },
                      { 7,  "mce",   "Machine Check Exception" },
                      { 8,  "cx8",   "CMPXCHG8B Instruction Supported" },
                      { 9,  "apic",  "On-chip CPIC Hardware Enabled" },
                      { 11, "sep",   "SYSENTER and SYSEXIT" },
                      { 12, "mtrr",  "Memory Type Range Registers" },
                      { 13, "pge",   "PTE Global Bit" },
                      { 14, "mca",   "Machine Check Architecture" },
                      { 15, "cmov",  "Conditional Move/Compare Instruction" },
                      { 16, "pat",   "Page Attribute Table" },
                      { 17, "pse",   "Page Size Extension" },
                      { 18, "psn",   "Processor Serial Number" },
                      { 19, "cflsh", "CFLUSH instruction" },
                      { 21, "ds",    "Debug Store" },
                      { 22, "acpi",  "Thermal Monitor and Clock Ctrl" },
                      { 23, "mmx",   "MMX Technology" },
                      { 24, "fxsr",  "FXSAVE/FXRSTOR" },
                      { 25, "sse",   "SSE Extensions" },
                      { 26, "sse2",  "SSE2 Extensions" },
                      { 27, "ss",    "Self Snoop" },
                      { 29, "tm",    "Therm. Monitor" },
                      { -1 }
                  };
        static struct {
            int bit;
            char *desc;;
            char *description;
        } cap_amd[] = {
                          { 22, "mmxext","MMX Technology (AMD Extensions)" },
                          { 30, "3dnowext","3Dnow! Extensions" },
                          { 31, "3dnow", "3Dnow!" },
                          { -1 }
                      };
        int i;

        regs = cpuid(1);
        printf("cpu family    : %d\n"
               "model        : %d\n"
               "stepping    : %d\n" ,
               (regs.eax >>  8) & 0xf,
               (regs.eax >>  4) & 0xf,
               regs.eax        & 0xf);

        printf("flags        :");
        for (i = 0; cap[i].bit >= 0; i++) {
            if (regs.edx & (1 << cap[i].bit)) {
                printf(" %s", cap[i].desc);
            }
        }
        for (i = 0; cap_amd[i].bit >= 0; i++) {
            if (amd_flags & (1 << cap_amd[i].bit)) {
                printf(" %s", cap_amd[i].desc);
            }
        }
        printf("\n");

        if (regs.edx & (1 << 4)) {
            int64_t tsc_start, tsc_end;
            struct timeval tv_start, tv_end;
            int usec_delay;

            tsc_start = rdtsc();

            struct time t_start, t_end;

            MESSAGE msg;
            msg.type = GET_RTC_TIME;
            msg.BUF= &t_start;

            send_recv(BOTH, TASK_SYS, &msg);

            //gettimeofday(&tv_start, NULL);
#ifdef	MISSING_USLEEP
            //sleep(1);
#else
            //usleep(100000);
#endif
            tsc_end = rdtsc();
            //gettimeofday(&tv_end, NULL);
            MESSAGE msg2;
            msg2.type = GET_RTC_TIME;
            msg2.BUF= &t_end;
            
            send_recv(BOTH, TASK_SYS, &msg2);

            usec_delay = 1000000 * (t_end.second + 1 - t_start.second);

            printf("cpu MHz        : %d\n",
                   (int)(tsc_end - tsc_start)/5 );
        }
    }

    printf("model name    : %s\n", model_name);

    exit(0);
}
