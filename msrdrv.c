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
static long msrdrv_ioctl(struct file *f, unsigned int ioctl_num);


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

/* Read/write handler */
static long msrdrv_ioctl(struct file *f, unsigned int ioctl_num)
{
    unsigned long long ia32_pmc0, ia32_pmc1, ia32_pmc2, ia32_pmc3, 
                        ia32_fixed_ctr0, ia32_fixed_ctr1, ia32_fixed_ctr2;
    struct MsrInOut *msrops;
    int i, j;
    if (ioctl_num != IOCTL_MSR_CMDS) {
            return 0;
    }
    
    /*
     * Notes on registers:
     *  ecx = MSR identifier:        unsigned int ecx;
     *  eax = low double word:       unsigned int eax;
     *  edx = high double word:      unsigned int edx;
     *  return value = quad word:    unsigned long long value;
     * 
     * Usage:
     *  write_msr(ecx, eax, edx);
     */

    /* Reset counters and start monitoring */
    write_msr(0x38f, 0x00, 0x00);           /* ia32_perf_global_ctrl: disable 4 PMCs (PrograMmable Counters) & 3 FFCs (Fixed Function Counters, count just one type of event) */
    write_msr(0xc1, 0x00, 0x00);            /* ia32_pmc0: zero value (35-5) */   
    write_msr(0xc2, 0x00, 0x00);            /* ia32_pmc1: zero value (35-5) */
    write_msr(0xc3, 0x00, 0x00);            /* ia32_pmc2: zero value (35-5) */
    write_msr(0xc4, 0x00, 0x00);            /* ia32_pmc3: zero value (35-5) */
    write_msr(0x309, 0x00, 0x00);           /* ia32_fixed_ctr0: zero value (35-17) */
    write_msr(0x30a, 0x00, 0x00);           /* ia32_fixed_ctr1: zero value (35-17) */
    write_msr(0x30b, 0x00, 0x00);           /* ia32_fixed_ctr2: zero value (35-17) */
    write_msr(0x186, 0x004201C2, 0x00);     /* ia32_perfevtsel1, UOPS_RETIRED.ALL (19-28)           NOTE - just user-land: 0x004101c2 */    
    write_msr(0x187, 0x0042010E, 0x00);     /* ia32_perfevtsel0, UOPS_ISSUED.ANY (19.22)            NOTE - just user-land: 0x0041010e */
    write_msr(0x188, 0x01C2010E, 0x00);     /* ia32_perfevtsel2, UOPS_ISSUED.ANY-stalls (19-22)     NOTE - just user-land: 0x01c1010e */
    write_msr(0x189, 0x004201A2, 0x00);     /* ia32_perfevtsel3, RESOURCE_STALLS.ANY (19-27)        NOTE - just user-land: 0x004101a2 */
    write_msr(0x38d, 0x222, 0x00);          /* ia32_perf_fixed_ctr_ctrl: ensure 3 FFCs enabled */
    write_msr(0x38f, 0x0f, 0x07);           /* ia32_perf_global_ctrl: enable 4 PMCs & 3 FFCs */

    for (j = 0 ; j <= 1000 ; j++) {
        j += 17;
    }
    printk("This is j: %d\n", j);

    /* Read counters */
    write_msr(0x38f, 0x00, 0x00);                              /* ia32_perf_global_ctrl: disable 4 PMCs & 3 FFCs */
    write_msr(0x38d, 0x00, 0x00);                              /* ia32_perf_fixed_ctr_ctrl: clean up FFC ctrls */
    ia32_pmc0 = read_msr(0xc1);             /* ia32_pmc0: read value (35-5) */    
    ia32_pmc1 = read_msr(0xc2);             /* ia32_pmc1: read value (35-5) */    
    ia32_pmc2 = read_msr(0xc3);             /* ia32_pmc2: read value (35-5) */    
    ia32_pmc3 = read_msr(0xc4);             /* ia32_pmc3: read value (35-5) */    
    ia32_fixed_ctr0 = read_msr(0x309);      /* ia32_fixed_ctr0: read value (35-17) */
    ia32_fixed_ctr1 = read_msr(0x30a);      /* ia32_fixed_ctr1: read value (35-17) */
    ia32_fixed_ctr2 = read_msr(0x30b);      /* ia32_fixed_ctr2: read value (35-17) */

    printk("uops retired:    %7lld\n", ia32_pmc0);
    printk("uops issued:     %7lld\n", ia32_pmc1);
    printk("stalled cycles:  %7lld\n", ia32_pmc2);
    printk("resource stalls: %7lld\n", ia32_pmc3);
    printk("instr retired:   %7lld\n", ia32_fixed_ctr0);
    printk("core cycles:     %7lld\n", ia32_fixed_ctr1);
    printk("ref cycles:      %7lld\n", ia32_fixed_ctr2);

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
