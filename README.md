Implementation of a Simple GPIO Driver for the Raspberry Pi

Supports interaction with the driver from userspace, exposed through the sysfs interface:

1. First make sure that sysfs is currently mounted. This is done by default on most systems but just in case:

mount -t sysfs sysfs /sys

Current implementation supports very limited functionalities, and is only limited to toggling the input/output mode of a pin, as well as writing to it. Future work intend to add edge-triggered IRQ handling 


