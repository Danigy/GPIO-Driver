#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function

#include <asm/io.h>

#define  DEVICE_NAME "gpio_driver"    //device appear as /dev/gpio_driver
#define  CLASS_NAME  "gpio_custom"        ///< The device class -- this is a character device driver
#define  GPIO_BASE 0x3f200000

#undef VERIFY_OCTAL_PERMISSIONS
#define VERIFY_OCTAL_PERMISSIONS(_mode) (_mode)

MODULE_LICENSE("GPL");

static char name[11] = "gpio_driver";
static int gpio_pin;	  //number indicating current activated gpio pin
static int gpio_mode;	  //0 or 1 indicating input or output
static int gpio_value;	  //current value gpio pin
static int retval; 		//sysfs_create_group return val

//create callback function to set gpio_pin
static ssize_t scan_pin(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	sscanf(buf, "%d", &gpio_pin);
	printk(KERN_INFO "gpio_pin set to: %d\n", gpio_pin);
	return strnlen(buf, sizeof(buf));
}

//callback function to show currently set pin
static ssize_t read_pin(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	return snprintf(buf, sizeof(buf), "%d\n", gpio_pin);	
}

//register kobject attribute using __ATTR macro
//name and access mode
//registers read(show) and set(write) callbacks
static struct kobj_attribute pin_attr = __ATTR(gpio_pin, 0600, read_pin, scan_pin);

//initialize list of attributes used to create attribute group
static struct attribute *gpio_attrs[] = {
	&pin_attr.attr,
	NULL,
};

//initialize attribute group
static struct attribute_group attr_group = {
	.name = name,
	.attrs = gpio_attrs,
};

static struct kobject *gpio_kobj;

static int __init init_driver(void){
	gpio_kobj = kobject_create_and_add("gpio_driver", kernel_kobj->parent);
	if(!gpio_kobj){
		printk(KERN_ALERT "Kobject creation failed\n");
		return -1;
	}
	retval = sysfs_create_group(gpio_kobj, &attr_group);
	if(retval){
		printk(KERN_ALERT "Kobject group creation failed\n");
		kobject_put(gpio_kobj);
		return retval;
	}
	gpio_pin = 69;
	printk(KERN_ALERT "Initialization finished, gpio_pin: %d\n", gpio_pin);	
	return 0;
}
	
static void __exit driver_exit(void){
	printk(KERN_INFO "Current GPIO pin is %d\n", gpio_pin);
	kobject_put(gpio_kobj);
	printk(KERN_INFO "Freed!\n");
}
module_init(init_driver);
module_exit(driver_exit);

