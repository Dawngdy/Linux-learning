#include "asm/uaccess.h"
#include "linux/kdev_t.h"
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/backing-dev.h>
#include <linux/shmem_fs.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/uio.h>

#include <linux/uaccess.h>

static unsigned int major;
static unsigned char hello_buf[100];
struct class *hello_class;

static	ssize_t hello_read (struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    long ret;
    unsigned long len = size > 100 ? 100: size;
    printk("%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);

    ret = copy_to_user(buf, hello_buf, len);  //将内核空间的数据copy到用户空间
    if (ret) 
    {
		printk("failed to copy_to_user\n");
		return ret;
	}

    //用户拿到hello_buf后，可以进行其他处理。
    return size;
}
//const char __user *buf中的__user 是一个空的宏，为了提醒用户，这里的buff不能直接使用。
static	ssize_t hello_write (struct file *filp, const char __user *buf, size_t size, loff_t *offset)
{
    long ret;
    unsigned long len = size > 100 ? 100: size;
    printk("%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
    ret = copy_from_user(hello_buf, buf, len);
	if (ret) 
    {
		printk("failed to copy_from_user\n");
		return ret;
	}

    return size;
}

static int hello_open (struct inode *node, struct file *filp)
{
    printk("%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
    return 0;

}

static int hello_release (struct inode *node, struct file *filp)
{
    printk("%s %s %d\n", __FILE__,__FUNCTION__,__LINE__);
    return 0;
}


/*1. create file_operate*/
static const struct file_operations hello_drv = {
    .owner      = THIS_MODULE,
	.read		= hello_read,
	.write		= hello_write,
    .open       = hello_open,
    .release    = hello_release,
};
/*2. register_chrdev*/

/*3. entry function*/
static int __init hello_dev_init(void)
{
	major = register_chrdev(0, "hello_dev", &hello_drv);

    //我们提供一些信息给系统（class创建，设备创建），系统的后台程序可以自动帮我们创建设备节点
	hello_class = class_create(THIS_MODULE, "hello_class");
    if (IS_ERR(hello_class)) 
    {
		printk("failed to allocate class\n");
		return PTR_ERR(hello_class);
	}
    device_create(hello_class, NULL, MKDEV(major, 0), NULL, "hello_dev");

    return 0;
}
/*4. exit function*/
static void __exit hello_exit(void)
{
    //退出函数中当然要销毁class和设备
    device_destroy(hello_class, MKDEV(major, 0));
    class_destroy(hello_class);
    
	unregister_chrdev(major, "hello_exit");
}

module_init(hello_dev_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");