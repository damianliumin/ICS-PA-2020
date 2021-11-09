#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Shdr Elf64_Shdr
# define Elf_Sym Elf64_Sym
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Shdr Elf32_Shdr
# define Elf_Sym Elf32_Sym
#endif

// ramdisk operations in ramdisk.c
size_t get_ramdisk_size();

int fs_open(const char *pathname, int flags, int mode); // 2
size_t fs_read(int fd, void *buf, size_t len); // 3
size_t fs_write(int fd, const void *buf, size_t len); // 4
int fs_close(int fd); // 7
size_t fs_lseek(int fd, size_t offset, int whence); // 8

/* ***** loader ***** */
static uintptr_t loader(PCB *pcb, const char *filename) {
  assert(get_ramdisk_size() != 0);
  int fd = fs_open(filename, 0, 0);
  assert(fd != -1);

  // read elf_header
  Elf_Ehdr head;
  fs_lseek(fd, 0, 0);
  fs_read(fd, &head, sizeof(Elf_Ehdr));
  //ramdisk_read(&head, st, sizeof(Elf_Ehdr));
  uint32_t ph_offset = head.e_phoff;
  uint32_t ph_entrysize = head.e_phentsize;
  uint32_t ph_cnt = head.e_phnum;
  // read segment header to load the program
  for(int i = 0 ;i < ph_cnt ;++i){
    Elf_Phdr ph;  // segment header
    fs_lseek(fd, ph_offset + i * ph_entrysize, 0);
    fs_read(fd, &ph, sizeof(Elf_Phdr));
    //ramdisk_read(&ph, st + ph_offset + i * ph_entrysize, sizeof(Elf_Phdr));
    if(ph.p_type != PT_LOAD) continue;
    uint32_t file_offset = ph.p_offset;
    uint32_t file_size = ph.p_filesz;
    uint32_t mem_size = ph.p_memsz;
    //char* mem_addr = 0;
    //mem_addr += ph.p_vaddr;
    /* load on page */
    void* va = 0;
    va = (void*)((char*)va + ph.p_vaddr);
    fs_lseek(fd, file_offset, 0);
    uint32_t rend = file_offset % PGSIZE;
    file_size += file_offset % PGSIZE;
    mem_size += file_offset % PGSIZE;

    va = (void*)((uintptr_t)va & (~0xfff));
    while(file_size){
      void *pa = new_page(1);
      uint32_t clr_size = (mem_size >= PGSIZE) ? PGSIZE : mem_size;
      memset(pa, 0, clr_size);
      map(&pcb->as, va, pa, 7);
      uint32_t read_size = (file_size >= PGSIZE) ? PGSIZE : file_size;
      fs_read(fd, (char*)pa + rend, read_size - rend);
      if(rend) rend = 0;
      file_size = (file_size >= PGSIZE) ? file_size - PGSIZE : 0;
      mem_size = (mem_size >= PGSIZE) ? mem_size - PGSIZE : 0;
      va = (void*)((char*)va + PGSIZE);
    }
    while(mem_size){
      void *pa = new_page(1);
      uint32_t clr_size = (mem_size >= PGSIZE) ? PGSIZE : mem_size;
      memset(pa, 0, clr_size);
      map(&pcb->as, va, pa, 7);
      mem_size -= clr_size;
      va = (void*)((char*)va + PGSIZE);
    }
    /* finish load on page */

    //fs_read(fd, mem_addr, file_size);
    //ramdisk_read(mem_addr, st + file_offset, file_size);
    //memset(mem_addr + file_size, 0, mem_size - file_size);
  }
  return head.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
  Area kstack;
  kstack.start = (void*)pcb;
  kstack.end = (void*)((char*)pcb + sizeof(PCB));
  protect(&pcb->as);
  // load user program and get entry
  void* entry = (void*)loader(pcb, filename);
  // set user context
  pcb->cp = ucontext(&pcb->as, kstack, entry);
  // assign user stack
  void* pf = new_page(8);
  void* va = (void*)((char*)pcb->as.area.end - 8 * PGSIZE);
  for(int i = 0 ;i <= 7 ;++i){
    map(&pcb->as, (void*)((char*)va + i * PGSIZE), (void*)((char*)pf + i * PGSIZE), 7);
  }
  // pass args to user program (in user stack)
  int argc = 0, envc = 0;
  if(argv == NULL) argc = 0;
  else{
    while(argv[argc] != NULL)
    ++ argc;
  }
  if(envp == NULL) envc = 0;
  else{
    while(envp[envc] != NULL)
    ++ envc;
  }

  int totlen = 0;
  for(int i = 0 ;i < argc ;++i)
    totlen += strlen(argv[i]) + 1;
  for(int i = 0 ;i < envc ;++i)
    totlen += strlen(envp[i]) + 1;
  
  char *strst = (char*)pf + 8 * PGSIZE - totlen;  // start of the string area

  uintptr_t tmp = (uintptr_t)strst - (3 + argc + envc) * sizeof(char*);
  char **argst = (char**)(tmp - tmp % sizeof(char*)); // start of the arg area
  
  argst[0] = (char*)(uintptr_t)argc;
  for(int i = 1 ;i <= argc ;++i){
    strcpy(strst, argv[i-1]);
    argst[i] = strst;
    strst += strlen(argv[i-1]) + 1;
  }
  argst[argc+1] = NULL;
  for(int i = 0 ;i < envc ;++i){
    strcpy(strst, envp[i]);
    argst[argc + 2 + i] = strst;
    strst += strlen(envp[i]) + 1;
  }
  argst[argc + 2 + envc] = NULL;

  // set GPRx for _start
  pcb->cp->GPRx = (uintptr_t)va - (uintptr_t)pf + (uintptr_t)argst;
}
