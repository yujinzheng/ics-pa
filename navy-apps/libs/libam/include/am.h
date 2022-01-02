#ifndef __AM_H__
#define __AM_H__

#define ARCH_H "navy.h"
#include "am-origin.h"

//extern char _pmem_start;
//#define PMEM_SIZE (128 * 1024 * 1024)
//#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

typedef uintptr_t PTE;

#if defined(__ARCH_X86_NEMU)
# define DEVICE_BASE 0x0
#else
# define DEVICE_BASE 0xa0000000
#endif

#define MMIO_BASE 0xa0000000

#define SERIAL_PORT     (DEVICE_BASE + 0x00003f8)
#define KBD_ADDR        (DEVICE_BASE + 0x0000060)
#define RTC_ADDR        (DEVICE_BASE + 0x0000048)
#define VGACTL_ADDR     (DEVICE_BASE + 0x0000100)
#define FB_ADDR         (MMIO_BASE   + 0x1000000)
#endif
