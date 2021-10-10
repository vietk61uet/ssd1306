#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/gpio.h>
 
#define SIGETX 44
 
#define REG_CURRENT_TASK _IOR('a','a',int32_t*)
 
#define IRQ_NO 62
 
/* Signaling to Application */
static struct task_struct *task = NULL;
static int signum = 0;
 
int32_t value = 0;
 
struct class *class_name;
struct device *device_name;
struct cdev my_cdev;
dev_t dev_1;
 
static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .unlocked_ioctl = etx_ioctl,
        .release        = etx_release,
};
 
//Interrupt handler for IRQ 62
static irqreturn_t irq_handler(int irq,void *dev_id) {
    struct kernel_siginfo info;
    pr_info("Shared IRQ: Interrupt Occurred");
    
    //Sending signal to app
    info.si_signo = SIGETX;
    info.si_code = SI_QUEUE;
    info.si_int = 1;
 
    if (task != NULL) {
        pr_info("Sending signal to app\n");
        if(send_sig_info(SIGETX, &info, task) < 0) {
            pr_info("Unable to send signal\n");
        }
    }
 
    return IRQ_HANDLED;
}
 
static int etx_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Button Opened...!!!\n");
    return 0;
}
 
static int etx_release(struct inode *inode, struct file *file)
{
    struct task_struct *ref_task = get_current();
    pr_info("Device File Button Closed...!!!\n");
    
    //delete the task
    if(ref_task == task) {
        task = NULL;
    }
    return 0;
}
 
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
    return 0;
}

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}
 
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (cmd == REG_CURRENT_TASK) {
        pr_info("REG_CURRENT_TASK\n");
        task = get_current();
        signum = SIGETX;
    }
    pr_info("ioctl OK");
    return 0;
}
 
 
static int __init etx_driver_init(void)
{
    int ret;

    gpio_request(44, "sysfs");
    gpio_direction_input(44);
    gpio_set_debounce(44, 100);
    gpio_export(44, false);
    ret = alloc_chrdev_region(&dev_1, 0, 1, "reboot_3");
    if (ret) {
        pr_info("Can not register major number\n");
        goto fail_reg;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev_1), MINOR(dev_1));

    cdev_init(&my_cdev, &fops);
    my_cdev.dev = dev_1;

    ret = cdev_add(&my_cdev, dev_1, 1);
    if (ret < 0) {
        pr_info("cdev_add error\n");
        return ret;
    }

    if ((class_name = class_create(THIS_MODULE,"reboot_3")) == NULL) {
        pr_info("create class failed\n");
        goto class_fail;
    }
    pr_info("create successfully class\n");

    if (device_create(class_name, NULL, dev_1, NULL, "reboot_3") == NULL) {
        pr_info("Create device failed\n");
        goto dev_fail;
    }
    pr_info("create device success\n");

    if (request_irq(IRQ_NO, (irq_handler_t) irq_handler, IRQF_TRIGGER_RISING, "reboot_3", NULL) != 0) {
        pr_info("my_device: cannot register IRQ ");
        pr_info("Failed to regster irq\n");
        goto irq;
    }
    return 0;

irq:
    free_irq(IRQ_NO,(void *)(irq_handler));
    cdev_del(&my_cdev);
    device_destroy(class_name, dev_1);
dev_fail:
    class_destroy(class_name);
class_fail:
    unregister_chrdev_region(dev_1, 1);
fail_reg:
    return -ENODEV;
}

static void __exit etx_driver_exit(void)
{
    pr_info("goodbye\n");
    cdev_del(&my_cdev);
    device_destroy(class_name, dev_1);
    class_destroy(class_name);
    unregister_chrdev_region(dev_1, 1);
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_AUTHOR("Hoang Nguyen <hoangson31096@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
