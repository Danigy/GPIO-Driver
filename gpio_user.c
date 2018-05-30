#include <linux/fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>

#define PERIPH 0x3f000000
#define GPIO_OFFSET 0x20000000
#define PAGE_SIZE 4096

void main(){
	int fd;
	if((fd = open("/dev/gpiomem", O_RDWR|O_SYNC)) < 0){
		printf("error opening address\n");
		close(fd);
		return -1;
	}
	unsigned int* addr;
	unsigned int* gpio_addr = PERIPH + GPIO_OFFSET;
	printf("%x\n", gpio_addr);
	
	addr = (unsigned int* )mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, gpio_addr);
	
	printf("GPIO mapped @ %x\n", addr);
	
	//int continue;
	//scanf("%d", continue);
	close(fd);
}
