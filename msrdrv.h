#ifndef _MG_MSRDRV_H
#define _MG_MSRDRV_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define DEV_NAME "msrdrv"
#define DEV_MAJOR 223
#define DEV_MINOR 0

#define MSR_VEC_LIMIT 32

#define IOCTL_MSR_CMDS _IO(DEV_MAJOR, 1)

enum MsrOperation {
    MSR_NOP   = 0,
    MSR_READ  = 1,
    MSR_WRITE = 2,
    MSR_STOP  = 3,
    MSR_RDTSC = 4
};

struct MsrInOut {
    unsigned int op;              // MsrOperation
    unsigned int ecx;             // msr identifier
    union {
        struct {
            unsigned int eax;     // low double word
            unsigned int edx;     // high double word
        };
        unsigned long long value; // quad word
    };
}; // msrdrv.h:27:1: warning: packed attribute is unnecessary for ‘MsrInOut’ [-Wpacked]

#endif