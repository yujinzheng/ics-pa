#ifndef __COMMON_H__
#define __COMMON_H__

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_CTE
//#define HAS_VME
//#define MULTIPROGRAM
//#define TIME_SHARING

#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <debug.h>

#if defined(__ISA_AM_NATIVE__)
# define EXCEPT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
# define EXCEPT_TYPE EM_X86_64
#elif defined(__ISA_MIPS32__)
# define EXCEPT_TYPE EM_MIPS
#elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
# define EXCEPT_TYPE EM_RISCV
#else
# define EXCEPT_TYPE -1
#error unsupported ISA __ISA__
#endif

#endif
