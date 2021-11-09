#ifndef __X86_RTL_H__
#define __X86_RTL_H__

#include <rtl/rtl.h>
#include "reg.h"

/* RTL pseudo instructions */
// rtl_lr: load register(given number) to dest
static inline def_rtl(lr, rtlreg_t* dest, int r, int width) {
  switch (width) {
    case 4: rtl_mv(s, dest, &reg_l(r)); return;
    case 1: rtl_host_lm(s, dest, &reg_b(r), 1); return;
    case 2: rtl_host_lm(s, dest, &reg_w(r), 2); return;
    default: assert(0);
  }
}
// rtl_sr: save src1 in register
static inline def_rtl(sr, int r, const rtlreg_t* src1, int width) {
  switch (width) {
    case 4: rtl_mv(s, &reg_l(r), src1); return;
    case 1: rtl_host_sm(s, &reg_b(r), src1, 1); return;
    case 2: rtl_host_sm(s, &reg_w(r), src1, 2); return;
    default: assert(0);
  }
}
// rtl_push:
static inline def_rtl(push, const rtlreg_t* src1, int width) {
  // esp <- esp - 4
  // M[esp] <- src1
  switch(width) {
    case 4: 
      rtl_subi(s, &reg_l(R_ESP), &reg_l(R_ESP), 4); 
      rtl_sm(s, &reg_l(R_ESP), 0, src1, 4);
      return;
    case 2:
      rtl_subi(s, &reg_l(R_ESP), &reg_l(R_ESP), 2);
      rtl_sm(s, &reg_l(R_ESP), 0, src1, 2);
      return;
    default: assert(0);
  }
}
// rtl_pop:
static inline def_rtl(pop, rtlreg_t* dest, int width) {
  // dest <- M[esp]
  // esp <- esp + 4
  switch(width) {
    case 4:
      rtl_lm(s, dest, &reg_l(R_ESP), 0, 4);
      rtl_addi(s, &reg_l(R_ESP), &reg_l(R_ESP), 4);
      return;
    case 2:
      rtl_lm(s, dest, &reg_l(R_ESP), 0, 2);
      rtl_addi(s, &reg_l(R_ESP), &reg_l(R_ESP), 2);
      return;
    default: assert(0);
  }
}
// rtl_is_sub_overflow: 
static inline def_rtl(is_sub_overflow, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
  // dest <- is_overflow(src1 - src2)
  switch(width){
    case 4:
      if((int)*src1 >= 0 && *src2 == 0x80000000){
        rtl_li(s, dest, 1);
        return;
      }
      if(((int)*src1 > 0 && (int)*src2 < 0 && (int)*res < 0)
        || ((int)*src1 < 0 && (int)*src2 > 0 && (int)*res  > 0))
        rtl_li(s, dest, 1);
      else 
        rtl_li(s, dest, 0);
      return;
    case 1:
      if((int8_t)*src1 >= 0 && (int8_t)*src2 == 0x80){
        rtl_li(s, dest, 1);
        return;
      }
      if(((int8_t)*src1 < 0 && (int8_t)*src2 > 0 && (int8_t)*res > 0)
        || ((int8_t)*src1 > 0 && (int8_t)*src2 < 0 && (int8_t)*res < 0))
        rtl_li(s, dest, 1);
      else 
        rtl_li(s, dest, 0);
      return;
    case 2:
      if((int16_t)*src1 >= 0 && (int16_t)*src2 == 0x8000){
        rtl_li(s, dest, 1);
        return;
      }
      if(((int16_t)*src1 < 0 && (int16_t)*src2 > 0 && (int16_t)*res > 0)
        || ((int16_t)*src1 > 0 && (int16_t)*src2 < 0 && (int16_t)*res < 0))
        rtl_li(s, dest, 1);
      else 
        rtl_li(s, dest, 0);
      return;
    default: assert(0);
  }
  
}
// rtl_is_sub_carry:
static inline def_rtl(is_sub_carry, rtlreg_t* dest,
    const rtlreg_t* src1, const rtlreg_t* src2) {
  // dest <- is_carry(src1 - src2)
  if(*src1 < *src2)
    rtl_li(s, dest, 1);
  else 
    rtl_li(s, dest, 0);
}
// rtl_is_add_overflow:
static inline def_rtl(is_add_overflow, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1, const rtlreg_t* src2, int width) {
  // dest <- is_overflow(src1 + src2)
  switch(width) {
    case 4:
      if(((int)*src1 > 0 && (int)*src2 > 0 && (int)*res < 0)
        || ((int)*src1 < 0 && (int)*src2 < 0 && (int)*res > 0))
        rtl_li(s, dest, 1);
      else
        rtl_li(s, dest, 0);
      return;
    case 1:
      if(((int8_t)*src1 > 0 && (int8_t)*src2 > 0 && (int8_t)*res < 0)
        || ((int8_t)*src1 < 0 && (int8_t)*src2 < 0 && (int8_t)*res > 0))
        rtl_li(s, dest, 1);
      else
        rtl_li(s, dest, 0);
      return;
    case 2:
      if(((int16_t)*src1 > 0 && (int16_t)*src2 > 0 && (int16_t)*res < 0)
        || ((int16_t)*src1 < 0 && (int16_t)*src2 < 0 && (int16_t)*res > 0))
        rtl_li(s, dest, 1);
      else
        rtl_li(s, dest, 0);
      return;
    default: assert(0);
  }
  
}
// rtl_is_add_carry:
static inline def_rtl(is_add_carry, rtlreg_t* dest,
    const rtlreg_t* res, const rtlreg_t* src1) {
  // dest <- is_carry(src1 + src2)
  if(*res < *src1)
    rtl_li(s, dest, 1);
  else 
    rtl_li(s, dest, 0);
  return;
}

// rtl_set_CF rtl_get_CF
#define def_rtl_setget_eflags(f) \
  static inline def_rtl(concat(set_, f), const rtlreg_t* src) { \
    if(*src != 0) \
      rtl_ori(s, &reg_eflags, &reg_eflags, 1u << f);\
    else \
      rtl_andi(s, &reg_eflags, &reg_eflags, ~(1u << f));\
  } \
  static inline def_rtl(concat(get_, f), rtlreg_t* dest) { \
    *dest = (reg_eflags & (1u << f)) >> f; \
  }

def_rtl_setget_eflags(CF)
def_rtl_setget_eflags(OF)
def_rtl_setget_eflags(ZF)
def_rtl_setget_eflags(SF)
// rtl_update_ZF:
static inline def_rtl(update_ZF, const rtlreg_t* result, int width) {
  // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
  *t0 = (0xffffffffu >> (8 * (4 - width))) & *result;
  *t0 = (*t0 == 0) ? 1 : 0;
  rtl_set_ZF(s, t0);
}
// rtl_update_SF:
static inline def_rtl(update_SF, const rtlreg_t* result, int width) {
  // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
  *t0 = (0xffffffffu >> (8 * (4 - width))) & *result;
  *t0 >>= (8 * width - 1);
  rtl_set_SF(s, t0);
}
// rtl_update_ZFSF:
static inline def_rtl(update_ZFSF, const rtlreg_t* result, int width) {
  rtl_update_ZF(s, result, width);
  rtl_update_SF(s, result, width);
}

#endif
