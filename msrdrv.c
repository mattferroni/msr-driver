#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include "msrdrv.h"

/* Comment this to disable debug */
#define _MG_DEBUG 0


#ifdef _MG_DEBUG
#define dprintk(args...) printk(args);
#else
#define dprintk(args...)
#endif

MODULE_LICENSE("Dual BSD/GPL");


/* Kernel module hooks */
static ssize_t msrdrv_read(struct file *f, char *b, size_t c, loff_t *o)
{
    return 0;
}

static ssize_t msrdrv_write(struct file *f, const char *b, size_t c, loff_t *o)
{
    return 0;
}

static int msrdrv_open(struct inode* i, struct file* f)
{
    return 0;
}

static int msrdrv_release(struct inode* i, struct file* f)
{
    return 0;
}


/* Custom method header */
static long msrdrv_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param);


/* Kernel module data structures */
dev_t msrdrv_dev;
struct cdev *msrdrv_cdev;
struct file_operations msrdrv_fops = {
    .owner =          THIS_MODULE,
    .read =           msrdrv_read,
    .write =          msrdrv_write,
    .open =           msrdrv_open,
    .release =        msrdrv_release,
    .unlocked_ioctl = msrdrv_ioctl,
    .compat_ioctl =   NULL,
};


/* Read/write operations */
static long long read_msr(unsigned int ecx) {
    unsigned int edx = 0, eax = 0;
    unsigned long long result = 0;
    __asm__ __volatile__("rdmsr" : "=a"(eax), "=d"(edx) : "c"(ecx));
    result = eax | (unsigned long long)edx << 0x20;
    dprintk(KERN_ALERT "Module msrdrv: Read 0x%016llx (0x%08x:0x%08x) from MSR 0x%08x\n", result, edx, eax, ecx)
    return result;
}

static void write_msr(int ecx, unsigned int eax, unsigned int edx) {
    dprintk(KERN_ALERT "Module msrdrv: Writing 0x%08x:0x%08x to MSR 0x%04x\n", edx, eax, ecx)
    __asm__ __volatile__("wrmsr" : : "c"(ecx), "a"(eax), "d"(edx));
}

static long long read_tsc(void)
{
    unsigned eax, edx;
    long long result;
    __asm__ __volatile__("rdtsc" : "=a"(eax), "=d"(edx));
    result = eax | (unsigned long long)edx << 0x20;
    dprintk(KERN_ALERT "Module msrdrv: Read 0x%016llx (0x%08x:0x%08x) from TSC\n", result, edx, eax)
    return result;
}


/* Read/write handler */
static long msrdrv_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param)
{
    struct MsrInOut *msrops;
    int i;
    if (ioctl_num != IOCTL_MSR_CMDS) {
            return 0;
    }
    msrops = (struct MsrInOut*)ioctl_param;
    for (i = 0 ; i <= MSR_VEC_LIMIT ; i++, msrops++) {
        switch (msrops->op) {
        case MSR_NOP:
            dprintk(KERN_ALERT "Module " DEV_NAME ": seen MSR_NOP command\n")
            break;
        case MSR_STOP:
            dprintk(KERN_ALERT "Module " DEV_NAME ": seen MSR_STOP command\n")
            goto label_end;
        case MSR_READ:
            dprintk(KERN_ALERT "Module " DEV_NAME ": seen MSR_READ command\n")
            msrops->value = read_msr(msrops->ecx);
            break;
        case MSR_WRITE:
            dprintk(KERN_ALERT "Module " DEV_NAME ": seen MSR_WRITE command\n")
            write_msr(msrops->ecx, msrops->eax, msrops->edx);
            break;
        case MSR_RDTSC:
            dprintk(KERN_ALERT "Module " DEV_NAME ": seen MSR_RDTSC command\n")
            msrops->value = read_tsc();
            break;
        default:
            dprintk(KERN_ALERT "Module " DEV_NAME ": Unknown option 0x%x\n", msrops->op)
            return 1;
        }
    }
    label_end:

    return 0;
}


/* Kernel module load/unload */
static int msrdrv_init(void)
{
    long int val;
    msrdrv_dev = MKDEV(DEV_MAJOR, DEV_MINOR);
    register_chrdev_region(msrdrv_dev, 1, DEV_NAME);
    msrdrv_cdev = cdev_alloc();
    msrdrv_cdev->owner = THIS_MODULE;
    msrdrv_cdev->ops = &msrdrv_fops;
    cdev_init(msrdrv_cdev, &msrdrv_fops);
    cdev_add(msrdrv_cdev, msrdrv_dev, 1);
    printk(KERN_ALERT "Module " DEV_NAME " loaded\n");
    return 0;
}

static void msrdrv_exit(void)
{
    long int val;
    cdev_del(msrdrv_cdev);
    unregister_chrdev_region(msrdrv_dev, 1);
    printk(KERN_ALERT "Module " DEV_NAME " unloaded\n");
}


/* Register kernel module */
module_init(msrdrv_init);
module_exit(msrdrv_exit);
