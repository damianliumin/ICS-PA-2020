#include <monitor/difftest.h>

static inline def_EHelper(lidt) {
  reg_idtr.limit = vaddr_read(*s->isa.mbase + s->isa.moff, 2);
  reg_idtr.base = vaddr_read(*s->isa.mbase + s->isa.moff + 2, 4);
  print_asm_template1(lidt);
}

static inline def_EHelper(lgdt) {
  reg_gdtr.limit = vaddr_read(*s->isa.mbase + s->isa.moff, 2);
  reg_gdtr.base = vaddr_read(*s->isa.mbase + s->isa.moff + 2, 4);
  print_asm_template1(lgdt);
}

static inline def_EHelper(ltr) {
  cpu.tr.selector = (uint16_t)*ddest;
  print_asm_template1(ltr);
}

static inline def_EHelper(mov_r2cr) {
  if(*ddest == 0)
    cpu.cr0 = *dsrc1;
  else if(*ddest == 3)
    cpu.cr3 = *dsrc1;
  else 
    assert(0);

  print_asm("movl %%%s,%%cr%d", reg_name(id_src1->reg, 4), id_dest->reg);
}

static inline def_EHelper(mov_cr2r) {
  if(*dsrc1 == 0)
    rtl_li(s, ddest, cpu.cr0);
  else if(*dsrc1 == 3)
    rtl_li(s, ddest, cpu.cr3);
  else 
    assert(0);
  operand_write(s, id_dest, ddest);


  print_asm("movl %%cr%d,%%%s", id_src1->reg, reg_name(id_dest->reg, 4));

#ifndef __DIFF_REF_NEMU__
  difftest_skip_ref();
#endif
}

void raise_intr(DecodeExecState *, uint32_t, vaddr_t);
static inline def_EHelper(int) {
  raise_intr(s, *ddest, cpu.pc + 2);
  print_asm("int %s", id_dest->str);

#ifndef __DIFF_REF_NEMU__
  difftest_skip_dut(1, 2);
#endif
}

static inline def_EHelper(iret) {
  rtl_pop(s, s0, 4);
  rtl_pop(s, &cpu.cs, 4);
  rtl_pop(s, &reg_eflags, 4);
  if((cpu.cs & 0x3) == 3){
    rtl_pop(s, s1, 4);
    rtl_pop(s, &cpu.ss, 4);
    rtl_mv(s, &cpu.esp, s1);
  }
  rtl_j(s, *s0);
  print_asm("iret");

#ifndef __DIFF_REF_NEMU__
  difftest_skip_ref();
#endif
}

uint32_t pio_read_l(ioaddr_t);
uint32_t pio_read_w(ioaddr_t);
uint32_t pio_read_b(ioaddr_t);
void pio_write_l(ioaddr_t, uint32_t);
void pio_write_w(ioaddr_t, uint32_t);
void pio_write_b(ioaddr_t, uint32_t);

static inline def_EHelper(in) {
  switch(id_dest->width){
    case 1: rtl_li(s, s0, pio_read_b(*dsrc1)); break;
    case 2: rtl_li(s, s0, pio_read_w(*dsrc1)); break;
    case 4: rtl_li(s, s0, pio_read_l(*dsrc1)); break;
    default: assert(0);
  }
  operand_write(s, id_dest, s0);
  print_asm_template2(in);
}

static inline def_EHelper(out) {
  switch(id_src1->width){
    case 1: pio_write_b(*ddest, *dsrc1); break;
    case 2: pio_write_w(*ddest, *dsrc1); break;
    case 4: pio_write_l(*ddest, *dsrc1); break;
    default: assert(0);
  }
  print_asm_template2(out);
}
