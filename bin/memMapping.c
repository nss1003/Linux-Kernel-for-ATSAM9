#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main()
{
     
        unsigned int *addr;

        int fd = open("/dev/mem",O_RDWR|O_SYNC);
        if(fd < 0)
        {
                printf("Can't open /dev/mem\n");
                return 1;
        }

	/*
	* Physical address  = 0xfffe000
	* Size = 16 KByte
	* Man mmap for more information
	*/
        addr = (unsigned int *) mmap(NULL, 16384, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xfffe0000);

        if(addr == NULL)
        {
                printf("Can't mmap\n");
                return 1;
        }
        else{
                printf("Virtual address = %p\n",addr);
		printf("*addr = %d\n", *addr);
	}
       return 0;
}
