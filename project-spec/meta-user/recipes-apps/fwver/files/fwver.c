#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/mman.h>
#include <stdint.h>

// 0: version
// 1: build time
// 2: RTC x10 ns
// 3: R/W

#define ADDR_SYS_REGS			(0xA0000000)
#define ADDR_BRAM				(0xA0010000)
#define ADDR_INT_CTRL			(0xA0020000)

#define SYS_VER_OFFSET			(0)
#define SYS_RTC_OFFSET			(1)
#define SYS_RW_OFFSET			(2)
#define SYS_BIT_INT_OFFSET		(3)

#define INT_MASK_OFFSET			(0)
#define INT_CNTR_OFFSET			(1)
#define INT_EVENT_OFFSET		(2)
#define INT_FIFO_OFFSET			(3)
#define INT_BLOCK_CTRL_OFFSET	(4)
#define INT_EVENT_CNTR_OFFSET	(5)

typedef union
{
	struct
	{
		uint64_t DebugVersionMinor			: 8;
		uint64_t DebugVersionMajor			: 8;
		uint64_t ReleaseVersionMinor		: 8;
		uint64_t ReleaseVersionMajor		: 8;
		uint64_t Seconds					: 6;
		uint64_t Minutes					: 6;
		uint64_t Hours						: 5;
		uint64_t Year						: 6;
		uint64_t Month						: 4;
		uint64_t Day						: 5;
	};

	uint64_t Value;
} RegDateTimeVersion;

typedef union
{
	struct
	{
		uint64_t GlobalInterrupt			: 1;
		uint64_t BitInterrupt				: 1;
		uint64_t 							: 62;
	};

	uint64_t Value;
} RegInterruptMask;

typedef union
{
	struct
	{
		uint64_t ResetEvents				: 1;
		uint64_t ResetInterruptCounter		: 1;
		uint64_t InterruptAck				: 1;
		uint64_t InterruptClear				: 1;
		uint64_t							: 60;
	};

	uint64_t Value;
} RegInterruptControl;




int main(int argc, char **argv)
{
	RegDateTimeVersion versionReg;
	RegInterruptMask intMaskReg;
	RegInterruptControl intCtrlReg;
	int cnt;
	uint32_t intread;

	int ddr_memory = open("/dev/mem", O_RDWR | O_SYNC);

	uint64_t* sysRegs = mmap(NULL, 0xFFFF, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, ADDR_SYS_REGS);
	uint64_t* intRegs = mmap(NULL, 0xFFFF, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, ADDR_INT_CTRL);

	versionReg.Value = sysRegs[SYS_VER_OFFSET];
    printf("FW build: %02i-%02i-%02i  %02i:%02i:%02i \r\n", versionReg.Day, versionReg.Month, versionReg.Year,
    		                                                 versionReg.Hours, versionReg.Minutes, versionReg.Seconds);


    // cat /sys/class/uio/uio4/name = pl_int
    int temp = open("/dev/uio4", O_RDWR | O_SYNC);

    intRegs[INT_BLOCK_CTRL_OFFSET] = 0x4;

    intRegs[INT_MASK_OFFSET] = 0xFFFF; //intMaskReg.Value;

    for (cnt = 1; cnt < 10; cnt++)
    {
    	sysRegs[SYS_BIT_INT_OFFSET] = 1;//cnt;

    	// wait for interrupt
    	read(temp, &intread, 4);


       	intRegs[INT_BLOCK_CTRL_OFFSET] = 0xF;
    	intRegs[INT_BLOCK_CTRL_OFFSET] = 0x4;

    	// re-enable interrupt through uio driver
    	write(temp, &intread, 4);
    }

	munmap((void*)sysRegs, 0xFFFF);
	munmap((void*)intRegs, 0xFFFF);

    return 0;
}

//#define MAP_SIZE 0xFFFFu
//#define MAP_MASK (MAP_SIZE - 1)
//
//int main()
//{
//	void* map_base;
//	uint64_t* virt_addr;
//	int fd;
//	off_t target = 0xa0000000u;
//
//	fd = open("/dev/mem", O_RDWR | O_SYNC);
//	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
//
//	virt_addr = map_base + (target & MAP_MASK);
//
//	uint64_t a = virt_addr[0];
//	uint64_t b = virt_addr[1];
//
//	virt_addr[1] = 0xABAB5555AAAABEEF;
//
//	uint64_t c = virt_addr[1];
//
////	uint64_t a = *((uint64_t*)virt_addr);
////	uint64_t b = *((uint64_t*)virt_addr + 1);
////	uint64_t c = *((uint64_t*)virt_addr + 2);
////	uint64_t d = *((uint64_t*)virt_addr + 3);
////	uint64_t e = *((uint64_t*)virt_addr + 4);
////	uint64_t f = *((uint64_t*)virt_addr + 5);
////
////	uint32_t sa = *((uint32_t*)virt_addr);
////	uint32_t sb = *((uint32_t*)virt_addr + 1);
////	uint32_t sc = *((uint32_t*)virt_addr + 2);
////	uint32_t sd = *((uint32_t*)virt_addr + 3);
////	uint32_t se = *((uint32_t*)virt_addr + 4);
////	uint32_t sf = *((uint32_t*)virt_addr + 5);
//
//	return 0;
//}
