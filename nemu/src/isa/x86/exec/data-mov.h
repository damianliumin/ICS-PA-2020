// exec_mov:
static inline def_EHelper(mov) {
  operand_write(s, id_dest, dsrc1);
  print_asm_template2(mov);
}
// exec_movs:
static inline def_EHelper(movs) {
  rtl_lm(s, s0, &reg_l(R_ESI), 0, id_dest->width);
  rtl_sm(s, &reg_l(R_EDI), 0, s0, id_dest->width);
  rtl_addi(s, &reg_l(R_ESI), &reg_l(R_ESI), id_dest->width);
  rtl_addi(s, &reg_l(R_EDI), &reg_l(R_EDI), id_dest->width);
  print_asm_template1(movs);
}
// exec_push:
static inline def_EHelper(push) {
  //uint32_t rec = cpu.esp;
  //printf("push - esp: %x val: %x\n", cpu.esp, *ddest);

  rtl_sext(s, s0, ddest, id_dest->width);
  if(id_dest->width == 1)
    id_dest->width = (s->isa.is_operand_size_16) ? 2 : 4;
  rtl_push(s, s0, id_dest->width);
  print_asm_template1(push);
}
// exec_pop:
static inline def_EHelper(pop) {
  rtl_pop(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template1(pop);
}
// exec_pusha
static inline def_EHelper(pusha) {
  if(s->isa.is_operand_size_16){
    rtl_lr(s, s0, R_ESP, 2);
    rtl_push(s, &(reg_l(R_EAX)), 2);
    rtl_push(s, &(reg_l(R_ECX)), 2);
    rtl_push(s, &(reg_l(R_EDX)), 2);
    rtl_push(s, &(reg_l(R_EBX)), 2);
    rtl_push(s, s0, 2);
    rtl_push(s, &(reg_l(R_EBP)), 2);
    rtl_push(s, &(reg_l(R_ESI)), 2);
    rtl_push(s, &(reg_l(R_EDI)), 2);
  } else {
    rtl_lr(s, s0, R_ESP, 4);
    rtl_push(s, &(reg_l(R_EAX)), 4);
    rtl_push(s, &(reg_l(R_ECX)), 4);
    rtl_push(s, &(reg_l(R_EDX)), 4);
    rtl_push(s, &(reg_l(R_EBX)), 4);
    rtl_push(s, s0, 4);
    rtl_push(s, &(reg_l(R_EBP)), 4);
    rtl_push(s, &(reg_l(R_ESI)), 4);
    rtl_push(s, &(reg_l(R_EDI)), 4);
  }
  print_asm("pusha");
}
// exec_popa
static inline def_EHelper(popa) {
  if(s->isa.is_operand_size_16){
    rtl_pop(s, &(reg_l(R_EDI)), 2);
    rtl_pop(s, &(reg_l(R_ESI)), 2);
    rtl_pop(s, &(reg_l(R_EBP)), 2);
    rtl_pop(s, s0, 2);
    rtl_pop(s, &(reg_l(R_EBX)), 2);
    rtl_pop(s, &(reg_l(R_EDX)), 2);
    rtl_pop(s, &(reg_l(R_ECX)), 2);
    rtl_pop(s, &(reg_l(R_EAX)), 2);
  } else {
    rtl_pop(s, &(reg_l(R_EDI)), 4);
    rtl_pop(s, &(reg_l(R_ESI)), 4);
    rtl_pop(s, &(reg_l(R_EBP)), 4);
    rtl_pop(s, s0, 4);
    rtl_pop(s, &(reg_l(R_EBX)), 4);
    rtl_pop(s, &(reg_l(R_EDX)), 4);
    rtl_pop(s, &(reg_l(R_ECX)), 4);
    rtl_pop(s, &(reg_l(R_EAX)), 4);
  }

  print_asm("popa");
}
// exec_leave
static inline def_EHelper(leave) {
  rtl_mv(s, &reg_l(R_ESP), &reg_l(R_EBP));
  rtl_pop(s, &reg_l(R_EBP), id_dest->width);
  print_asm("leave");
}
// exec_cltd
static inline def_EHelper(cltd) {
  if (s->isa.is_operand_size_16) {
    rtl_sext(s, s0, &reg_l(R_AX), 2);
    rtl_sari(s, s0, s0, 16);
    rtl_sr(s, R_DX, s0, 2);
  }
  else {
    rtl_mv(s, s0, &reg_l(R_EAX));
    rtl_sari(s, s0, s0, 31);
    rtl_sr(s, R_EDX, s0, 4);
  }

  print_asm(s->isa.is_operand_size_16 ? "cwtl" : "cltd");
}
// exec_cwtl
static inline def_EHelper(cwtl) {
  if (s->isa.is_operand_size_16) {
    rtl_shli(s, &reg_l(R_EAX), &reg_l(R_EAX), 24);
    rtl_sari(s, &reg_l(R_EAX), &reg_l(R_EAX), 24);
  }
  else {
    rtl_shli(s, &reg_l(R_EAX), &reg_l(R_EAX), 16);
    rtl_sari(s, &reg_l(R_EAX), &reg_l(R_EAX), 16);
  }
  print_asm(s->isa.is_operand_size_16 ? "cbtw" : "cwtl");
}
// exec_movsx
static inline def_EHelper(movsx) {
  id_dest->width = s->isa.is_operand_size_16 ? 2 : 4;
  rtl_sext(s, ddest, dsrc1, id_src1->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(movsx);
}
// exec_movzx
static inline def_EHelper(movzx) {
  id_dest->width = s->isa.is_operand_size_16 ? 2 : 4;
  operand_write(s, id_dest, dsrc1);
  print_asm_template2(movzx);
}
// exec_lea
static inline def_EHelper(lea) {
  rtl_addi(s, ddest, s->isa.mbase, s->isa.moff);
  operand_write(s, id_dest, ddest);
  print_asm_template2(lea);
}

static inline def_EHelper(xchg) {
  rtl_mv(s, s0, &reg_l(R_EAX));
  cpu.eax = *ddest;
  //rtl_mv(s, &reg_l(R_EAX), ddest);
  operand_write(s, id_dest, s0);
  print_asm_template1(xchg);
}
