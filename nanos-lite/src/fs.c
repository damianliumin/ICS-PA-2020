#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t cur_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_FB, FD_DISPINFO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* VFS */
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write, 0},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write, 0},
  [FD_DISPINFO]={"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write, 0},
#include "files.h"
};
#define FD_ENTRY_NUM  sizeof(file_table) / sizeof(Finfo)

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

// initialize cur_offset
void init_cur(){
  for(int i = 0 ;i < FD_ENTRY_NUM ;++i)
    file_table[i].cur_offset = file_table[i].disk_offset;
}

int fs_open(const char *pathname, int flags, int mode){ // 2
  for(int i = 0 ;i < FD_ENTRY_NUM ;++i)
    if(strcmp(pathname, file_table[i].name) == 0){
      file_table[i].cur_offset = file_table[i].disk_offset;
      return i;
    }
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len){ // 3
  if( fd != FD_EVENTS && fd != FD_DISPINFO && 
    file_table[fd].cur_offset + len > file_table[fd].disk_offset + file_table[fd].size)
    len = file_table[fd].disk_offset + file_table[fd].size - file_table[fd].cur_offset;

  if(file_table[fd].read == 0)  
    ramdisk_read(buf, file_table[fd].cur_offset, len);
  else if(fd == FD_EVENTS || fd == FD_DISPINFO) // ret value is at most len, rather than len
    return file_table[fd].read(buf, file_table[fd].cur_offset, len);  
  else
    file_table[fd].read(buf, file_table[fd].cur_offset, len);  
  file_table[fd].cur_offset += len;

  return len;
}

size_t fs_write(int fd, const void *buf, size_t len){ // 4
  if(fd != FD_STDOUT && fd != FD_STDERR &&
    file_table[fd].cur_offset + len > file_table[fd].disk_offset + file_table[fd].size)
    len = file_table[fd].disk_offset + file_table[fd].size - file_table[fd].cur_offset;

  if(file_table[fd].write == 0)  
    ramdisk_write(buf, file_table[fd].cur_offset, len);
  else
    file_table[fd].write(buf, file_table[fd].cur_offset, len);  
  file_table[fd].cur_offset += len;
  return len;
}

int fs_close(int fd){ // 7
  file_table[fd].cur_offset = file_table[fd].disk_offset;
  return 0;
}

size_t fs_lseek(int fd, size_t offset, int whence){ // 8
  switch(whence){
    case SEEK_SET: file_table[fd].cur_offset = file_table[fd].disk_offset + offset; break;
    case SEEK_CUR: file_table[fd].cur_offset += offset; break;
    case SEEK_END: file_table[fd].cur_offset = file_table[fd].disk_offset + file_table[fd].size + offset; break;
    default: assert(0);
  }
  return file_table[fd].cur_offset - file_table[fd].disk_offset;
}

void init_fs() {
  init_cur();
  int whinfo[2];
  dispinfo_read(whinfo, 0, 8);
  file_table[FD_FB].size = whinfo[0] * whinfo[1] * 4;
}
