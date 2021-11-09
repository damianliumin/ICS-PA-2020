#ifndef __RTL_BASIC_H__
#define __RTL_BASIC_H__

#include "c_op.h"
#include <memory/vaddr.h>

/* RTL basic instructions */
// src2 is a reg, pass by address
#define def_rtl_compute_reg(name) \
  static inline def_rtl(name, rtlreg_t* dest, const rtlreg_t* src1, const rtlreg_t* src2) { \
    *dest = concat(c_, name) (*src1, *src2); \
  }
// src2 is an imm, pass by value
#define def_rtl_compute_imm(name) \
  static inline def_rtl(name ## i, rtlreg_t* dest, const rtlreg_t* src1, const sword_t imm) { \
    *dest = concat(c_, name) (*src1, imm); \
  }

#define def_rtl_compute_reg_imm(name) \
  def_rtl_compute_reg(name) \
  def_rtl_compute_imm(name) \

// compute (basic operations for reg and imm)
// example: rtl_add, rtl_addi
def_rtl_compute_reg_imm(add)
def_rtl_compute_reg_imm(sub)
def_rtl_compute_reg_imm(and)
def_rtl_compute_reg_imm(or)
def_rtl_compute_reg_imm(xor)
def_rtl_compute_reg_imm(shl)
def_rtl_compute_reg_imm(shr)
def_rtl_compute_reg_imm(sar)

// rtl_setrelop: relation operations for unsigned
static inline def_rtl(setrelop, uint32_t relop, rtlreg_t *dest,
    const rtlreg_t *src1, const rtlreg_t *src2) {
  *dest = interpret_relop(relop, *src1, *src2);
}
// rtl_setrelopi: relation oprations for signed
static inline def_rtl(setrelopi, uint32_t relop, rtlreg_t *dest,
    const rtlreg_t *src1, sword_t imm) {
  *dest = interpret_relop(relop, *src1, imm);
}

// mul/div
// example: rtl_mul_lo, rtl_mul_loi
def_rtl_compute_reg(mul_lo)
def_rtl_compute_reg(mul_hi)
def_rtl_compute_reg(imul_lo)
def_rtl_compute_reg(imul_hi)
def_rtl_compute_reg(div_q)
def_rtl_compute_reg(div_r)
def_rtl_compute_reg(idiv_q)
def_rtl_compute_reg(idiv_r)

// rtl_div64_q: quotient of 64bit / 32bit
static inline def_rtl(div64_q, rtlreg_t* dest,
    const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  uint64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
  uint32_t divisor = (*src2);
  *dest = dividend / divisor;
}
// rtl_div64_r: remain of 64bit / 32bit
static inline def_rtl(div64_r, rtlreg_t* dest,
    const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  uint64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
  uint32_t divisor = (*src2);
  *dest = dividend % divisor;
}
// rtl_idiv64_q: signed quotient
static inline def_rtl(idiv64_q, rtlreg_t* dest,
    const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  int64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
  int32_t divisor = (*src2);
  *dest = dividend / divisor;
}
// rtl_idv64_r: signed remain
static inline def_rtl(idiv64_r, rtlreg_t* dest,
    const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  int64_t dividend = ((uint64_t)(*src1_hi) << 32) | (*src1_lo);
  int32_t divisor = (*src2);
  *dest = dividend % divisor;
}

// memory
// rtl_lm: dest <- [addr+offset]
static inline def_rtl(lm, rtlreg_t *dest, const rtlreg_t* addr, word_t offset, int len) {
  *dest = vaddr_read(*addr + offset, len);
}
// rtl_sm: [addr+offset] <- src1
static inline def_rtl(sm, const rtlreg_t* addr, word_t offset, const rtlreg_t* src1, int len) {
  vaddr_write(*addr + offset, *src1, len);
}
// rtl_lms: dest <- [addr+offset] (with higher bits in dest set to sign)
static inline def_rtl(lms, rtlreg_t *dest, const rtlreg_t* addr, word_t offset, int len) {
  word_t val = vaddr_read(*addr + offset, len);
  switch (len) {
    case 4: *dest = (sword_t)(int32_t)val; return;
    case 1: *dest = (sword_t)( int8_t)val; return;
    case 2: *dest = (sword_t)(int16_t)val; return;
    default: assert(0);
  }
}
// rtl_host_lm: read mem in host
static inline def_rtl(host_lm, rtlreg_t* dest, const void *addr, int len) {
  switch (len) {
    case 4: *dest = *(uint32_t *)addr; return;
    case 1: *dest = *( uint8_t *)addr; return;
    case 2: *dest = *(uint16_t *)addr; return;
    default: assert(0);
  }
}
// rtl_host_sm: write in host mem
static inline def_rtl(host_sm, void *addr, const rtlreg_t *src1, int len) {
  switch (len) {
    case 4: *(uint32_t *)addr = *src1; return;
    case 1: *( uint8_t *)addr = *src1; return;
    case 2: *(uint16_t *)addr = *src1; return;
    default: assert(0);
  }
}

// control
// rtl_j: set jump target
static inline def_rtl(j, vaddr_t target) {
  s->jmp_pc = target;
  s->is_jmp = true;
}
// rtl_jr: set jump target (addr of target given)
static inline def_rtl(jr, rtlreg_t *target) {
  s->jmp_pc = *target;
  s->is_jmp = true;
}
// rtl_jrelop: set jump target on contion
static inline def_rtl(jrelop, uint32_t relop,
    const rtlreg_t *src1, const rtlreg_t *src2, vaddr_t target) {
  bool is_jmp = interpret_relop(relop, *src1, *src2);
  if (is_jmp) rtl_j(s, target);
}
#endif
