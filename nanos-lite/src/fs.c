#include <fs.h>

size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
void set_screen_size(int width, int height);
size_t fb_write(const void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);

typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

typedef struct {
    char *name;
    size_t size;
    size_t disk_offset;
    ReadFn read;
    WriteFn write;
    size_t open_offset;
} Finfo;

enum {
    FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENT, FD_FBCTL, FD_FB
};

size_t invalid_read(void *buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
        [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
        [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
        [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
        [FD_EVENT]  = {"/dev/events", 0, 0, events_read, invalid_write},
        [FD_FBCTL]  = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
        [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},

#include "files.h"
};

int fs_open(const char *pathname, int flags, int mode) {
    int file_num = sizeof(file_table) / sizeof(file_table[0]);
    for (int idx = 0; idx < file_num; idx++) {
        if (strcmp(pathname, file_table[idx].name) == 0) {
            file_table[idx].open_offset = 0;
            return idx;
        }
    }
    Log("Can not find file with file name: %s", *pathname);
    assert(0);
}

size_t fs_read(int fd, void *buf, size_t len) {
    Finfo *file_info = &file_table[fd];
    if (file_info->read != NULL) {
        return file_info->read(buf, 0, len);
    }
    size_t disk_offset = file_info->disk_offset;
    size_t file_size = file_info->size;
    size_t open_offset = file_info->open_offset;
//    printf("===disk_offset: %d, file_size: %d, open_offset: %d, len: %d\n", disk_offset, file_size, open_offset, len);
    if (open_offset + len >= file_size) {
        len = file_size - open_offset;
    }
    int result = ramdisk_read(buf, disk_offset + open_offset, len);
    file_info->open_offset += result;
//    printf("---disk_offset: %d, file_size: %d, open_offset: %d, len: %d\n", disk_offset, file_size, file_info->open_offset, len);
    return result;
}

size_t fs_write(int fd, const void *buf, size_t len) {
    Finfo *file_info = &file_table[fd];
    size_t disk_offset = file_info->disk_offset;
    size_t file_size = file_info->size;
    size_t open_offset = file_info->open_offset;
    if (file_info->write != NULL) {
        return file_info->write(buf, disk_offset + open_offset, len);
    }
    if (open_offset + len > file_size) {
        Log("Write data is more than file's size!, file_name: %s, file_size: %d, len: %d", file_info->name, file_size,
            len);
        assert(0);
    }
    int result = ramdisk_write(buf, open_offset + disk_offset, len);
    file_info->open_offset += result;
    return result;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
    if (fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) {
        return 0;
    }
    Finfo *file_info = &file_table[fd];
    size_t file_size = file_info->size;
    size_t open_offset = file_info->open_offset;
//    printf("===mode: %d, offset: %d, open_offset: %d\n", whence, offset, open_offset);
    switch (whence) {
        case SEEK_SET:
            if (offset > file_size) {
                Log("SEEK_SET offset is large than file_size: file_name: %s", file_info->name);
                assert(0);
            }
            file_info->open_offset = offset;
            return file_info->open_offset;
        case SEEK_CUR:
            if (offset + open_offset > file_size) {
                Log("SEEK_CUR offset is large than file_size, file_name: %s", file_info->name);
                assert(0);
            }
            file_info->open_offset += offset;
            return file_info->open_offset;
        case SEEK_END:
            if (file_size + open_offset > file_size) {
                Log("SEEK_END offset is large than file_size, file_name: %s", file_info->name);
                assert(0);
            }
            file_info->open_offset = file_size + offset;
            return file_info->open_offset;
        default:
            Log("Unsupport seek mode: %d, file_name: %s", whence, file_info->name);
            return -1;
    }
}

int fs_close(int fd) {
    return 0;
}

typedef AM_GPU_CONFIG_T gpu_config;
/**
 * 对文件系统进行初始化，获取屏幕的相关信息，然后设置/dev/fb的文件大小
 *
 */
void init_fs() {
    gpu_config gpuConfig;
    ioe_read(AM_GPU_CONFIG, &gpuConfig);
    set_screen_size(gpuConfig.width, gpuConfig.height);
    file_table[FD_FB].size = gpuConfig.width * gpuConfig.height * sizeof(uint32_t);
}
