#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_TARGET_AM)
static uint8_t *pmem = NULL;
#else
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

uint8_t *guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }

paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
    word_t ret = host_read(guest_to_host(addr), len);
    return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
    host_write(guest_to_host(addr), len, data);
}

void init_mem() {
#if   defined(CONFIG_TARGET_AM)
    pmem = malloc(CONFIG_MSIZE);
    assert(pmem);
#endif
#ifdef CONFIG_MEM_RANDOM
    uint32_t *p = (uint32_t *) pmem;
    int i;
    for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i++) {
        p[i] = rand();
    }
#endif
    Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]",
        (paddr_t) CONFIG_MBASE, (paddr_t) CONFIG_MBASE + CONFIG_MSIZE);
}

void mem_trace_read(paddr_t addr, word_t value, int len) {
#ifdef CONFIG_ITRACE_MEM_SCOPE
    if (CONFIG_ITRACE_MEM_START > addr || CONFIG_ITRACE_MEM_STOP < addr) {
            return;
        }
#endif
    printf("---- Memory read\taddr: 0x%08x\tvalue: 0x%08x\tlen: %d ----\n", addr, value, len);
}

void mem_trace_write(paddr_t addr, word_t data, int len) {
#ifdef CONFIG_ITRACE_MEM_SCOPE
    if (CONFIG_ITRACE_MEM_START > addr || CONFIG_ITRACE_MEM_STOP < addr) {
            return;
        }
#endif
    printf("++++ Memory write\taddr: 0x%08x\tdata: 0x%08x\tlen: %d ++++\n", addr, data, len);
}

word_t paddr_read(paddr_t addr, int len) {
    word_t result;
    if (likely(in_pmem(addr))) {
        result = pmem_read(addr, len);
#ifdef CONFIG_ITRACE_MEM
        mem_trace_read(addr, result, len);
#endif
        return result;
    }
    MUXDEF(CONFIG_DEVICE, return mmio_read(addr, len),
           panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR ") at pc = " FMT_WORD,
                   addr, CONFIG_MBASE, CONFIG_MBASE + CONFIG_MSIZE, cpu.pc));
}

void paddr_write(paddr_t addr, int len, word_t data) {
    if (likely(in_pmem(addr))) {
        pmem_write(addr, len, data);
#ifdef CONFIG_ITRACE_MEM
        mem_trace_write(addr, data, len);
#endif
        return;
    }
    MUXDEF(CONFIG_DEVICE, mmio_write(addr, len, data),
           panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR ") at pc = " FMT_WORD,
                   addr, CONFIG_MBASE, CONFIG_MBASE + CONFIG_MSIZE, cpu.pc));
}
