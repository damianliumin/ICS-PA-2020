#include <am.h>
#include <x86.h>
#include <klib.h>

#define NR_IRQ         256     // IDT size
#define SEG_KCODE      1
#define SEG_KDATA      2
#define NR_SEG 6
static SegDesc gdt[NR_SEG] = {};
static TSS32 tss = {};

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

#include <klib.h>
Context* __am_irq_handle(Context *c) {
  __am_get_cur_as(c);
  if(tss.esp0 == 0)   // kernel
    c->cs &= ~0x3;
  else                // user
    c->cs |= 0x3;
  tss.esp0 = 0;
  if (user_handler) {
    Event ev = {0};
    switch (c->irq) {
      case 0x81: ev.event = EVENT_YIELD; break;
      case 0x80: ev.event = EVENT_SYSCALL; break;
      case 32: ev.event = EVENT_IRQ_TIMER; break;

      default: ev.event = EVENT_ERROR; break;
    }
    c = user_handler(ev, c);
    assert(c != NULL);
  }
  __am_switch(c);
  if((c->cs & 0x3) == 3){
    tss.esp0 = (uint32_t)(c + 1);
  }
  // printf("eax: %x\n", c);
  return c;
}

// init cte



bool cte_init(Context*(*handler)(Event, Context*)) {
  static GateDesc32 idt[NR_IRQ];

  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i]  = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecnull, DPL_KERN);
  }

  // ----------------------- interrupts ----------------------------
  idt[32]   = GATE32(STS_IG, KSEL(SEG_KCODE), __am_irq0,    DPL_KERN);
  // ---------------------- system call ----------------------------
  idt[0x80] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecsys,  DPL_USER);
  idt[0x81] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vectrap, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  // initialize GDT
  gdt[1] = SEG32(STA_X | STA_R,   0,      0xffffffff, DPL_KERN);
  gdt[2] = SEG32(STA_W,           0,      0xffffffff, DPL_KERN);
  gdt[3] = SEG32(STA_X | STA_R,   0,      0xffffffff, DPL_USER);
  gdt[4] = SEG32(STA_W,           0,      0xffffffff, DPL_USER);
  gdt[5] = SEG16(STS_T32A,     &tss, sizeof(tss) - 1, DPL_KERN);
  set_gdt(gdt, sizeof(gdt[0]) * NR_SEG);

  // initialize TSS
  tss.ss0 = KSEL(2);
  set_tr(KSEL(5));

  return true;
}


Context* kcontext(Area kstack, void (*entry)(void *), void *arg) { // arg considered as pointer
  Context* ctx = (Context*)((char*)kstack.end - sizeof(Context) - 8);
  ctx->eip = (uintptr_t)entry;
  ctx->esp = (uintptr_t)kstack.end - 8;
  ctx->cs = KSEL(1);
  ctx->cr3 = NULL;
  ctx->eflags |= (1 << 9); // set IF = 1
  uintptr_t *addr_for_arg = (uintptr_t*)((uintptr_t)ctx + sizeof(Context) - 4);
  addr_for_arg[0] = (uintptr_t)arg;
  
  return ctx;
}

void yield() {
  asm volatile("int $0x81");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
