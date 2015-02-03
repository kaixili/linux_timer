#include <linux/module.h>
#include <linux/device.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/timex.h>
#include <linux/rtc.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("lkx810");

int _timer = 0;
int interval = 10;
int time = 100;
module_param(interval, uint, S_IRUGO|S_IWUSR);
module_param(time, uint, S_IRUGO|S_IWUSR);

struct timer_list mytimer;

static ssize_t Timer_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
        char output[20];
        int len;
        len = strlen(output);
        if (*ppos != 0) return 0;
        snprintf(output, sizeof(output), "%d\n", _timer);   /*itoa*/
	    copy_to_user(buf,output,len);
        *ppos =len;
        return len;
}

static int Timer_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
        char dev_buf[20];
        int len;
        if (count == 0) return 0;
        if (copy_from_user(dev_buf,buf,count)) return -EINVAL;
        _timer = simple_strtoul(dev_buf, NULL, 10);    /*atoi*/
        mod_timer(&mytimer, jiffies + msecs_to_jiffies(interval));
        len = strlen(dev_buf);
        return len;
}



static struct file_operations dev_fops = {   
        .owner = THIS_MODULE,  
        .write = Timer_write,  
        .read = Timer_read,
}; 

static struct miscdevice misc = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "timer",
        .fops = &dev_fops,
};

static void timer_main(unsigned long a)
{
        if (_timer >= time || _timer <0){
                /*print UTC time*/
                struct timex txc;
                struct rtc_time tm;
                do_gettimeofday(&txc.time);
                rtc_time_to_tm(txc.time.tv_sec,&tm);
                printk(KERN_INFO "TIME %d-%d %d:%d:%d \n",tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
                //
                _timer = 0;
        }
        _timer +=1;
        mod_timer(&mytimer, jiffies + msecs_to_jiffies(interval));
}

static int __init mytimer_init(void)
{
        setup_timer(&mytimer, timer_main,(unsigned long) 0);
        mytimer.expires = jiffies + msecs_to_jiffies(interval);
        printk(KERN_INFO "\n\nTimer Set.\n");

        misc_register(&misc);  /*misc_dev register:chrdev(10,"timer",&dev_fops)*/
        add_timer(&mytimer);
        return 0;
}
    
static void __exit mytimer_exit(void)
{
        del_timer(&mytimer);
        misc_deregister(&misc);
        printk(KERN_INFO "Timer Delected.\n");
}

module_init(mytimer_init);
module_exit(mytimer_exit);
