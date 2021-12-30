#include <proc.h>
#include <elf.h>
#include <stdio.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

/**
 * 根据文件名，将文件加载到内存中
 *
 * @param file
 * @return
 */
uintptr_t init_program_mem(PCB *pcb, const char *filename) {
//    printf("fd name is: %s\n", filename);
    Elf_Ehdr elf_head;
    int fd = fs_open(filename, 0, 0);
//    printf("fd is: %d\n", fd);
    fs_read(fd, &elf_head, sizeof(Elf_Ehdr));

    // elf_ident中存储的是ELF的魔数和其他信息
    // elf_ident的魔数在第一位，取值为0x7f
    // elf_ident的第二到四位存储的是“ELF”
    if (elf_head.e_ident[0] != 0x7f || elf_head.e_ident[1] != 'E' || elf_head.e_ident[2] != 'L' ||
        elf_head.e_ident[3] != 'F') {
        printf("Not a ELF file\n");
        assert(0);
    }

    if (EXCEPT_TYPE != elf_head.e_machine) {
        printf("Inappropriate ISA\n");
        assert(0);
    }

    Elf_Phdr ph_eh_dr[elf_head.e_phnum];
    fs_lseek(fd, elf_head.e_phoff, SEEK_SET);
    fs_read(fd, ph_eh_dr, sizeof(Elf_Phdr) * elf_head.e_phnum);

    for (int idx = 0; idx < elf_head.e_phnum; idx++) {
        if (ph_eh_dr[idx].p_type == PT_LOAD) {
            fs_lseek(fd, ph_eh_dr[idx].p_offset, SEEK_SET);
            fs_read(fd, (void *)ph_eh_dr[idx].p_vaddr, ph_eh_dr[idx].p_memsz);
            if (ph_eh_dr[idx].p_memsz == ph_eh_dr[idx].p_filesz) {
                continue;
            }
            memset((void *)(ph_eh_dr[idx].p_vaddr + ph_eh_dr[idx].p_filesz), 0,
                   ph_eh_dr[idx].p_memsz - ph_eh_dr[idx].p_filesz);
        }
    }

    return elf_head.e_entry;
}

/**
 * 把用户的程序加载到正确的内存位置
 *
 * @param pcb 待定
 * @param filename 需要加载的文件名
 * @return elf文件入口
 */
static uintptr_t loader(PCB *pcb, const char *filename) {
    return init_program_mem(pcb, filename);
}

void naive_uload(PCB *pcb, const char *filename) {
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = 0x%x\n", entry);
    ((void (*)()) entry)();
}

