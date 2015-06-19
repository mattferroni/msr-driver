#ifndef _MG_MSRDRV_H
#define _MG_MSRDRV_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define DEV_NAME "msrdrv"
#define DEV_MAJOR 223
#define DEV_MINOR 0

#define MSR_VEC_LIMIT 32

#define IOCTL_MSR_CMDS _IO(DEV_MAJOR, 1)

#endif