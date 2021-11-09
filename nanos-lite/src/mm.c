#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *ret = pf;
  pf = (void*)((char*)pf + nr_page * PGSIZE);
  return ret;
}

static inline void* pg_alloc(int n) {
  size_t nr_page = n / PGSIZE;
  void* ret = new_page(nr_page);
  memset(ret, 0, n);
  return ret;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
extern PCB *current;

int mm_brk(uintptr_t brk) {
  if(current->max_brk == 0) {   // this max_brk is to be initialized with new program
    current->max_brk = brk;
    if(brk % PGSIZE == 0){
    void *pa = new_page(1);
    map(&current->as, (void*)current->max_brk, pa, 0);
    }
  }
  
  if(brk > current->max_brk){
    if(brk / PGSIZE > current->max_brk / PGSIZE){
      void* va = (void*)(current->max_brk | (PGSIZE - 1)) + 1;
      while(brk >= (uintptr_t)va){
        void *pa = new_page(1);
        map(&current->as, va, pa, 0);
        va = (char*)va + PGSIZE;
      }
    }
    current->max_brk = brk;
  }

  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

  #ifdef HAS_VME
  vme_init(pg_alloc, free_page);
  #endif
}
