#include <linux/kernel.h>    
#include <linux/module.h>  
#include <linux/device.h>   
#include <mach/platform.h>         
#include <linux/platform_device.h>  
#include <linux/types.h>    
#include <linux/fs.h>     
#include <linux/ioctl.h>    
#include <linux/cdev.h>    
#include <linux/delay.h>    
#include <linux/uaccess.h>  
#include <linux/init.h>   
#include <linux/gpio.h> 
#include <asm/io.h>
#include <mach/platform.h>

#define uchar unsigned char
#define uint  unsigned int

#define pinCLK 	18
#define pinCS 	23
#define pinDIN 	24
#define DRIVER_NAME "MAX7219"


int major;
struct gpio_chip *gpiochip;
static dev_t MAX7219_dev_id;				//device id
static struct class* MAX7219_class;	//device class
static struct cdev MAX7219_cdev;	//character device

static int state = 0;	//0->closed 1->open

struct GpioRegisters
{
    uint32_t GPFSEL[6];
    uint32_t Reserved1;
    uint32_t GPSET[2];
    uint32_t Reserved2;
    uint32_t GPCLR[2];
};

struct GpioRegisters *s_pGpioRegisters;

uchar codeDisp[38][8]={
	{0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C},//0
	{0x10,0x18,0x14,0x10,0x10,0x10,0x10,0x10},//1
	{0x7E,0x2,0x2,0x7E,0x40,0x40,0x40,0x7E},//2
	{0x3E,0x2,0x2,0x3E,0x2,0x2,0x3E,0x0},//3
	{0x8,0x18,0x28,0x48,0xFE,0x8,0x8,0x8},//4
	{0x3C,0x20,0x20,0x3C,0x4,0x4,0x3C,0x0},//5
	{0x3C,0x20,0x20,0x3C,0x24,0x24,0x3C,0x0},//6
	{0x3E,0x22,0x4,0x8,0x8,0x8,0x8,0x8},//7
	{0x0,0x3E,0x22,0x22,0x3E,0x22,0x22,0x3E},//8
	{0x3E,0x22,0x22,0x3E,0x2,0x2,0x2,0x3E},//9
	{0x8,0x14,0x22,0x3E,0x22,0x22,0x22,0x22},//A
	{0x3C,0x22,0x22,0x3E,0x22,0x22,0x3C,0x0},//B
	{0x3C,0x40,0x40,0x40,0x40,0x40,0x3C,0x0},//C
	{0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x0},//D
	{0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x7C},//E
	{0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x40},//F
	{0x3C,0x40,0x40,0x40,0x40,0x44,0x44,0x3C},//G
	{0x44,0x44,0x44,0x7C,0x44,0x44,0x44,0x44},//H
	{0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x7C},//I
	{0x3C,0x8,0x8,0x8,0x8,0x8,0x48,0x30},//J
	{0x0,0x24,0x28,0x30,0x20,0x30,0x28,0x24},//K
	{0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7C},//L
	{0x81,0xC3,0xA5,0x99,0x81,0x81,0x81,0x81},//M
	{0x0,0x42,0x62,0x52,0x4A,0x46,0x42,0x0},//N
	{0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C},//O
	{0x3C,0x22,0x22,0x22,0x3C,0x20,0x20,0x20},//P
	{0x1C,0x22,0x22,0x22,0x22,0x26,0x22,0x1D},//Q
	{0x3C,0x22,0x22,0x22,0x3C,0x24,0x22,0x21},//R
	{0x0,0x1E,0x20,0x20,0x3E,0x2,0x2,0x3C},//S
	{0x0,0x3E,0x8,0x8,0x8,0x8,0x8,0x8},//T
	{0x42,0x42,0x42,0x42,0x42,0x42,0x22,0x1C},//U
	{0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18},//V
	{0x0,0x49,0x49,0x49,0x49,0x2A,0x1C,0x0},//W
	{0x0,0x41,0x22,0x14,0x8,0x14,0x22,0x41},//X
	{0x41,0x22,0x14,0x8,0x8,0x8,0x8,0x8},//Y
	{0x0,0x7F,0x2,0x4,0x8,0x10,0x20,0x7F},//Z
	{0x8,0x7F,0x49,0x49,0x7F,0x8,0x8,0x8},//中
	{0xFE,0xBA,0x92,0xBA,0x92,0x9A,0xBA,0xFE},//国
};

static void bcm2835_gpio_fsel(int pin, int functionCode)
{
    int registerIndex = pin / 10;
    int bit = (pin % 10) * 3;

    unsigned oldValue = s_pGpioRegisters-> GPFSEL[registerIndex];
    unsigned mask = 0b111 << bit;
    printk("Changing function of GPIO%d from %x to %x\n", 
           pin,
           (oldValue >> bit) & 0b111,
           functionCode);

    s_pGpioRegisters-> GPFSEL[registerIndex] = 
        (oldValue & ~mask) | ((functionCode << bit) & mask);
}

static void bcm2835_gpio_set(int pin)
{
    s_pGpioRegisters-> GPSET[pin / 32] = (1 << (pin % 32));      
}

static void bcm2835_gpio_clr(int pin)
{
	s_pGpioRegisters-> GPCLR[pin / 32] = (1 << (pin % 32));
}

void Delay_xms(uint x)
{
	uint i,j;
	for(i=0;i<x;i++)
		for(j=0;j<50000;j++);
}

void Write_Max7219_byte(uchar DATA)         
{
    uchar i;   
    bcm2835_gpio_clr(pinCS);
	for(i=8;i>=1;i--)
    {
    	bcm2835_gpio_clr(pinCLK);
    	if((DATA&0x80) >> 7)
    		bcm2835_gpio_set(pinDIN);
    	else
    		bcm2835_gpio_clr(pinDIN);
        DATA <<= 1;
        bcm2835_gpio_set(pinCLK);
    }                                 
}

void Write_Max7219(uchar address,uchar dat)
{ 
	bcm2835_gpio_clr(pinCS);
	Write_Max7219_byte(address);           //writing address
    Write_Max7219_byte(dat);               //writing data 
	bcm2835_gpio_set(pinCS);
}

void Init_MAX7219(void)
{
	 Write_Max7219(0x09, 0x00);       //encoding with BCD
	 Write_Max7219(0x0a, 0x03);       //luminance
	 Write_Max7219(0x0b, 0x07);       //scanning bound
	 Write_Max7219(0x0c, 0x01);       //mode: normal
	 Write_Max7219(0x0f, 0x00);       
}

/*@brief set the gpio function input/output
*@parameter pin The selected gpio pin 
*@parameter functionCode 
*/

static int MAX7219_open(struct inode *inode, struct file *flip)
{
	printk("Open the MAX7219 device!\n");
	if(state != 0)
	{
		printk("The file is opened!\n");	
		return -1;
	}
	state++;
	printk("Open MAX7219 successfully!\n");
	return 0;
}

static int MAX7219_release(struct inode *inode, struct file *flip)
{
	printk("Close the MAX7219 device!\n");
	if(state == 1)
	{
		state = 0;
		printk("Close the file successfully!\n");
		return 0;
	}
	else
	{
		printk("The file has closed!\n");
		return -1;
	}
}

static ssize_t MAX7219_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	printk("Write %s into the MAX7219\n", buf);
	int ret, i;
	char ch;
	if(len == 0)
		return 0;
	
	if(copy_from_user(&ch, (void *)buf, 1))
		ret = -EFAULT;
	else
	{
		int index;
		if(ch >= '0' && ch <= '9')
			index = ch - '0';
		else if(ch >= 'A' && ch <= 'Z')
			index = ch - 'A' + 10;
		else if(ch >= 'a' && ch <= 'z')
			index = ch - 'z' + 10;
		else
			index = 36;	//unknown display 中
		printk("Write character %c, index=%d\n", ch, index);
		for(i=0;i<8;i++)
			Write_Max7219(i, codeDisp[index][i]);
		Delay_xms(1000);

		ret = 1;	//write a character
	}
	printk("Write character2 %c\n", ch);
	return ret;
}

static struct file_operations MAX7219_cdev_fops = {
	.owner = THIS_MODULE,
	.open = MAX7219_open,
	.write = MAX7219_write,
	.release = MAX7219_release,
};

static int MAX7219_init(void)
{
	int ret;

	MAX7219_dev_id = MKDEV(major, 0);
	if(major)	//static allocate 
		ret = register_chrdev_region(MAX7219_dev_id, 1, DRIVER_NAME);
    else	//dynamic allocate
    {
        ret = alloc_chrdev_region(&MAX7219_dev_id, 0, 1, DRIVER_NAME);
        major = MAJOR(MAX7219_dev_id);
    }

    if(ret < 0)
    	return ret;

	cdev_init(&MAX7219_cdev, &MAX7219_cdev_fops);	//initialize character dev
	cdev_add(&MAX7219_cdev, MAX7219_dev_id, 1);				//register character device
	MAX7219_class = class_create(THIS_MODULE, DRIVER_NAME);	//create a class
	device_create(MAX7219_class, NULL, MAX7219_dev_id, NULL, DRIVER_NAME);	//create a dev

	s_pGpioRegisters = (struct GpioRegisters *)__io_address(GPIO_BASE);

	printk("address = %x\n", (int)__io_address(GPIO_BASE));
	
	//gpio configure
	bcm2835_gpio_fsel(pinCLK, 1);
    bcm2835_gpio_fsel(pinCS, 1);
    bcm2835_gpio_fsel(pinDIN, 1);

    //initialize the MAX7219
    Init_MAX7219();

	printk("MAX7219 init successfully");
	return 0;
}

void MAX7219_exit(void)
{
	bcm2835_gpio_clr(pinCLK);
	bcm2835_gpio_clr(pinCS);
	bcm2835_gpio_clr(pinDIN);

	device_destroy(MAX7219_class, MAX7219_dev_id);
	class_destroy(MAX7219_class);
	unregister_chrdev_region(MAX7219_dev_id, 1);
	printk("MAX7219 exit successfully\n");
}

module_init(MAX7219_init);
module_exit(MAX7219_exit);

MODULE_DESCRIPTION("Raspberry MAX7219 Driver");
MODULE_AUTHOR("Jack<wang_kejie@foxmail.com>");
MODULE_LICENSE("GPL");