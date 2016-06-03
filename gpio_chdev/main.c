#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/ioctl.h>  
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/stat.h>  

void Delay_xms(uint x)
{
    uint i,j;
    for(i=0;i<x;i++)
        for(j=0;j<50000;j++);
}

int main(int argc, char **argv)  
{  
    int fd; 
    int ret; 

    fd = open("/dev/MAX7219", O_WRONLY);  
    if (fd < 0) 
    {  
        fprintf(stderr, "Fail to open /dev/MAX7219!\n");
        exit(1);
    } 
    char buff[1];
    int i=0;
    for(i=0;i<26;i++)
    {
        buff[0] = 'a' + i;
        if((ret = write(fd, buff, 1))<0)
        {
            fprintf(stderr, "Fail to write /dev/MAX7219! %d\n", ret);
            break;
        }
        Delay_xms(1000);
    }
    for(i=0;i<10;i++)
    {
        buff[0] = '0' + i;
        if((ret = write(fd, buff, 1))<0)
        {
            fprintf(stderr, "Fail to write /dev/MAX7219! %d\n", ret);
            break;
        }
        Delay_xms(1000);
    }
    fprintf(stdout, "Write /dev/MAX7219 successfully! %d\n", ret);
    close(fd);  
    return 0;  
}  