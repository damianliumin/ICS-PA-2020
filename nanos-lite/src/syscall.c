#include <common.h>
#include <sys/time.h>
#include <proc.h>
#include "syscall.h"

/* ***** File System ***** */
int fs_open(const char *pathname, int flags, int mode); // 2
size_t fs_read(int fd, void *buf, size_t len); // 3
size_t fs_write(int fd, const void *buf, size_t len); // 4
int fs_close(int fd); // 7
size_t fs_lseek(int fd, size_t offset, int whence); // 8

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
void switch_boot_pcb();

void sys_exit(int status){  // 0 
  char *argv[2] = {"bin/nterm", 0};
  context_uload(current, "/bin/nterm", argv, NULL);
  switch_boot_pcb();
  yield();
  // halt(status);
}

void sys_yield(){  // 1
  yield();
}

static int first_time = 1;
int mm_brk(uintptr_t brk);
// void* new_page(size_t nr_page);
/* WARNING! ONLY FOR ONE USER PROFRAM NOW !!!*/
void sys_brk(intptr_t addr, intptr_t increment){ // 9
  if(first_time){
    first_time = 0;
    mm_brk(addr - increment);
  }
  mm_brk(addr);
}

int sys_gettimeofday(struct timeval* tv){
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec = us / 1000000;
  tv->tv_usec = us % 1000000;
  return 0;
}

int sys_execve(const char* filename, char *const argv[], char *const envp[]){
  if(fs_open(filename, 0, 0) == -1)
    return -2;
  context_uload(current, filename, argv, envp);
  current->max_brk = 0;
  switch_boot_pcb();

  yield();
  assert(0);
  return -1;
}


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case SYS_exit: sys_exit(a[1]); break;
    case SYS_yield: sys_yield(); c->GPRx = 0; break;
    case SYS_open: c->GPRx = fs_open((char*)a[1], a[2], a[3]); break;
    case SYS_read: assert(a[1] != 0); c->GPRx = fs_read(a[1], (void*)a[2], a[3]); break;
    case SYS_write: c->GPRx = fs_write(a[1], (void*)a[2], a[3]); break;
    case SYS_close: c->GPRx = fs_close(a[1]); break;
    case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
    case SYS_brk: sys_brk(a[1], a[2]); c->GPRx = 0; break;
    case SYS_gettimeofday: c->GPRx = sys_gettimeofday((struct timeval*)a[1]); break;
    case SYS_execve: c->GPRx = sys_execve((char*)a[1], (char**)a[2], (char**)a[3]); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

}
