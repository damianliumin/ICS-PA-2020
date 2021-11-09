#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0xc0000000)

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f; 
  pgfree_usr = pgfree_f;
  kas.ptr = pgalloc_f(PGSIZE);
  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }
  set_cr3(kas.ptr);
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;
  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  //printf("protect: as->ptr - %x\n", as->ptr);
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->cr3 = (vme_enable ? (void *)get_cr3() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->cr3 != NULL) {
    set_cr3(c->cr3);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  uint32_t vaddr = (uint32_t)va;
  uint32_t pde_idx = (vaddr >> 22) & 0x3ff;
  uint32_t pte_idx = (vaddr >> 12) & 0x3ff;
  PTE* pde = (PTE*)as->ptr;
  // if((uintptr_t)va >= 0x40000000)
    // printf("map pde: %x, &as: %x, va: %x -> pa: %x\n", pde, as, va, pa);
  if(!(pde[pde_idx] & PTE_P))
    pde[pde_idx] = (uint32_t)pgalloc_usr(1 * PGSIZE) | PTE_P;
  PTE* pte = (PTE*)(pde[pde_idx] & ~0xfff);
  if((pte[pte_idx] & PTE_P) == 0)
    pte[pte_idx] = (uint32_t)pa | PTE_P;
}

Context* ucontext(AddrSpace *as, Area kstack, void *entry) {
  //protect(as); // this part is now implemented in context_uload

  Context* ctx = (Context*)((char*)kstack.end - sizeof(Context));
  ctx->eip = (uintptr_t)entry;
  ctx->cr3 = as->ptr;
  ctx->cs = USEL(3);
  ctx->ss3 = USEL(4);
  ctx->eflags |= (1 << 9); // set IF = 1
  return ctx;
}
