#include "cc.h"


static inline def_EHelper(test) {
  rtl_and(s, s0, ddest, dsrc1);
  rtl_set_CF(s, rz);
  rtl_set_OF(s, rz);
  rtl_update_ZFSF(s, s0, id_dest->width);
  print_asm_template2(test);
}

static inline def_EHelper(and) {
  rtl_sext(s, s0, dsrc1, id_src1->width);
  rtl_and(s, ddest, ddest, s0);
  rtl_set_CF(s, rz);
  rtl_set_OF(s, rz);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(and);
}

static inline def_EHelper(xor) {
  rtl_sext(s, s0, dsrc1, id_src1->width);
  rtl_xor(s, ddest, ddest, s0);
  rtl_set_CF(s, rz);
  rtl_set_OF(s, rz);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(xor);
}

static inline def_EHelper(or) {
  rtl_sext(s, s0, dsrc1, id_src1->width);
  rtl_or(s, ddest, ddest, s0);
  rtl_set_CF(s, rz);
  rtl_set_OF(s, rz);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(or);
}

static inline def_EHelper(sar) {
  // unnecessary to update CF and OF in NEMU
  if(id_dest->width != 4)
    rtl_sext(s, ddest, ddest, id_dest->width);
  rtl_sar(s, ddest, ddest, dsrc1);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(sar);
}

static inline def_EHelper(shl) {
  // unnecessary to update CF and OF in NEMU
  rtl_shl(s, ddest, ddest, dsrc1);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(shl);
}

static inline def_EHelper(shr) {
  // unnecessary to update CF and OF in NEMU
  if(id_dest->width != 4)
    rtl_andi(s, ddest, ddest, 0xffffffffu >> ((4 - id_dest->width) * 8));
  rtl_shr(s, ddest, ddest, dsrc1);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(shr);
}


static inline def_EHelper(setcc) {
  uint32_t cc = s->opcode & 0xf;
  rtl_setcc(s, ddest, cc);
  operand_write(s, id_dest, ddest);
  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

static inline def_EHelper(not) {
  rtl_not(s, ddest, ddest);
  print_asm_template1(not);
}

/* The following functions are defined by myself */
/*
static inline def_EHelper(rcl) {
  TODO();
  print_asm_template2(rcl);
}

static inline def_EHelper(rcr){
  TODO();
  print_asm_template2(rcr);
}
*/
static inline def_EHelper(rol){
  rtl_andi(s, dsrc1, dsrc1, 0xff);
  if(*dsrc1 == 1){
    rtl_msb(s, s0, ddest, id_dest->width);
    rtl_get_CF(s, s1);
    if(*s0 != *s1){
      rtl_li(s, s2, 1);
      rtl_set_OF(s, s2);
    } else {
      rtl_li(s, s2, 0);
      rtl_set_OF(s, s2);
    }
  }
  for(int i = 1 ;i <= *dsrc1 ;++i){    
    rtl_msb(s, s0, ddest, id_dest->width);
    rtl_shli(s, ddest, ddest, 1);
    rtl_add(s, ddest, ddest, s0);
  }
  operand_write(s, id_dest, ddest);
  rtl_msb(s, s0, ddest, id_dest->width);
  rtl_get_CF(s, s1);
  rtl_xor(s, s2, s0, s1);
  rtl_set_CF(s, s2);
  print_asm_template2(rol);
}
/*
static inline def_EHelper(ror){
  TODO();
  print_asm_template2(ror);
}
*/

static inline def_EHelper(shrd){
  assert(id_dest->width == 4);
  //printf("goal: %x imm: %x src: %x\n", *ddest, *dsrc1, *dsrc2);
  *dsrc1 %= 32;
  if(*dsrc1 != 0 && ((*ddest >> (*dsrc1 - 1)) & 0x1) == 0){
    rtl_li(s, s0, 1);
    rtl_set_CF(s, s0);
  }
  *ddest >>= *dsrc1;
  *s1 = (*dsrc2 << (32 - *dsrc1)) & (0xffffffff << *dsrc1);
  *ddest |= *s1;
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  //printf("result: %x\n", *ddest);
  print_asm_template3(shrd);
}

