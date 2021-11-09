#include <cpu/exec.h>
#include "local-include/rtl.h"

void raise_intr(DecodeExecState *s, uint32_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  if((cpu.cs & 0x3) == 3){
    uint32_t gate1 = vaddr_read(reg_gdtr.base + cpu.tr.selector , 4);
    uint32_t gate2 = vaddr_read(reg_gdtr.base + cpu.tr.selector + 4, 4);
    uint32_t tss_addr = ((gate1 & 0xffff0000) >> 16) + 
            ((gate2 & 0xff) << 16) + (gate2 & 0xff000000);
    // printf("tss_addr nemu: %x\n", tss_addr);
    // printf("gdt base: %x\n", cpu.gdtr.base);
    
    uint32_t tss_esp0 = vaddr_read(tss_addr + 4, 4);

    // printf("esp0: %x\n", tss_esp0);
    if(tss_esp0 != 0){
      rtl_mv(s, s0, &cpu.esp);
      rtl_li(s, &cpu.esp, tss_esp0); // ksp
      rtl_push(s, &cpu.ss, 4);  // push ss
      rtl_push(s, s0, 4);  // push esp
    }
  }

  uint32_t gate1 = vaddr_read(reg_idtr.base + 8 * NO, 4);
  uint32_t gate2 = vaddr_read(reg_idtr.base + 8 * NO + 4, 4);
  uint32_t address = (gate1 & 0xffff) + (gate2 & 0xffff0000);
  rtl_push(s, &reg_eflags, 4);
  cpu.eflags &= ~(1 << 9);  // set IF to 0
  rtl_push(s, &cpu.cs, 4);
  rtl_push(s, &ret_addr, 4);
  rtl_j(s, address);
}

#define IRQ_TIMER 32

void query_intr(DecodeExecState *s) {
  // debug
  // cpu.INTR = false;

  if (cpu.INTR && (cpu.eflags & (1 << 9)) != 0) {
    cpu.INTR = false;
    raise_intr(s, IRQ_TIMER, cpu.pc); // warning: ret_addr
    update_pc(s);
  }
}
