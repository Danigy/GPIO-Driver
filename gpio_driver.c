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

#define CLEAR_MASK ~0b111 
#define SET_OUTPUT(reg, off) (reg & (CLEAR_MASK << off)) | (1 << off)
#define SET_INPUT(reg, off) (reg & (CLEAR_MASK << off))

MODULE_LICENSE("GPL");

static const char name[5] = "gpio\0";
static const char name_pinval[6] = "value\0";
static char pin_pre[20] = "gpio"; //2 additional bytes for gpio num, 1 for nullbyte

static int gpio_pin;	  //number indicating current activated gpio pin
static int gpio_mode;	  //0 or 1 indicating input or output
static int gpio_value;	  //current value gpio pin
static int retval; 		//sysfs_create_group return val

static void __iomem *gpio;


//read and set current gpio pin
static ssize_t set_pin(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	sscanf(buf, "%d", &gpio_pin);
	printk(KERN_INFO "gpio_pin set to: %d\n", gpio_pin);
		
	
	return strnlen(buf, sizeof(buf));
}

static ssize_t read_pin(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	return snprintf(buf, sizeof(gpio_pin), "%d\n", gpio_pin);	
}

//read and set output mode
static ssize_t set_mode(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	int gpfunsel_line = gpio_pin / 10;
	int gpfunsel_off = gpio_pin % 10 * 3;
	int32_t GPFUNSEL;

	sscanf(buf, "%d", &gpio_mode);
	
	GPFUNSEL = (uint32_t)ioread32((uint32_t *)gpio + gpfunsel_line); 
	GPFUNSEL = (gpio_mode) ? SET_OUTPUT(GPFUNSEL, gpfunsel_off) : SET_INPUT(GPFUNSEL, gpfunsel_off);
	
	iowrite32(GPFUNSEL, gpio + gpfunsel_line);

	return sizeof(gpio_mode);
}

static ssize_t read_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	return snprintf(buf, sizeof(gpio_mode), "%d\n", gpio_mode);	
}


//register kobject attribute using __ATTR macro
//name and access mode
//registers read(show) and set(write) callbacks 
static struct kobj_attribute gpio_pin_attr = __ATTR(gpio_pin, 0600, read_pin, set_pin);
static struct kobj_attribute gpio_mode_attr = __ATTR(gpio_mode, 0600, read_mode, set_mode);

static struct attribute *gpio_attrs[] = {
	&gpio_pin_attr.attr,
	&gpio_mode_attr.attr,
	NULL,
};

// #define ATTRIBUTE_GROUPS(_name)                 \
// static const struct attribute_group _name##_group = {       \
//     .attrs = _name##_attrs,                 \
// };                              \

// __ATTRIBUTE_GROUPS(_name)

ATTRIBUTE_GROUPS(gpio);

static struct kobject* gpio_kobj;

static int __init init_driver(void){
	gpio_kobj = kobject_create_and_add("gpio_driver", kernel_kobj->parent);
	if(!gpio_kobj){
		printk(KERN_ALERT "Kobject creation failed\n");
		return -1;
	}
	retval = sysfs_create_group(gpio_kobj, &gpio_group);
	if(retval){
		printk(KERN_ALERT "Kobject group creation failed\n");
		kobject_put(gpio_kobj);
		return retval;
	}
	gpio = ioremap(GPIO_BASE, 4096);
	if(gpio == NULL){
		printk(KERN_ALERT "IORemap Failed\n");
	}

	
	gpio_pin = 0;
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
