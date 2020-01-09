#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/timer.h>  /*timer*/
#include <asm/uaccess.h>  /*jiffies*/
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

struct tasklet_struct task_t;
struct workqueue_struct *mywork;
struct work_struct work;

static void task_fuc(unsigned long data)
{
	if (in_interrupt()) {
		printk("%s in interrupt handler!\n", __FUNCTION__);
	}
}

//工作队列处理函数
static void mywork_fuc (struct work_struct *work)
{
	if (in_interrupt()) {
		printk("%s in interrupt handler!\n", __FUNCTION__);		
	}

	msleep(2);
	printk("%s in process handle!\n", __FUNCTION__);
}

static irqreturn_t irq_function(int irq, void *dev_id)
{
	tasklet_schedule(&task_t);
	schedule_work(&work);
	if (in_interrupt()) {
		printk ("%s in interrupt handle!\n", __FUNCTION__);
	}

	printk("key_irq:%d\n",irq);
	return IRQ_HANDLED;
}

static int __init tiny4412_Key_irq_test_init(void)
{
	int err = 0;
	int irq_num1;
	int data_t = 100;
	//创建新队列和新工作者线程
	mywork = create_workqueue("my work");
	//初始化
	INIT_WORK(&work, mywork_fuc);
	//调度指定队列
	queue_work(mywork, &work);
	tasklet_init(&task_t, task_fuc, data_t);
	printk("irq_key init\n");
	irq_num1 = gpio_to_irq(EXYNOS4_GPX3(2));
	err = request_irq(irq_num1, irq_function, IRQF_TRIGGER_FALLING, "tiny4412_key1", (void *)"key1");
	if (err != 0) {
		free_irq(irq_num1, (void*)"key1");
		return -1;
	}
	return 0;
}

static void __exit tiny4412_Key_irq_test_exit(void)
{
	int irq_num1;
	printk("irq_key exit\n");
	irq_num1 = gpio_to_irq(EXYNOS4_GPX3(2));
	destroy_workqueue(mywork);
	free_irq(irq_num1, (void*)"key1");
}

module_init(tiny4412_Key_irq_test_init);
module_exit(tiny4412_Key_irq_test_exit);

MODULE_LICENSE("GPL");

