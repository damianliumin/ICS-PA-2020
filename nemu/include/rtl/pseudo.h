#ifndef __RTL_PSEUDO_H__
#define __RTL_PSEUDO_H__

#ifndef __RTL_RTL_H__
#error "Should be only included by <rtl/rtl.h>"
#endif

/* RTL pseudo instructions */
// rtl_li: dest <- imm + 0
static inline def_rtl(li, rtlreg_t* dest, const rtlreg_t imm) {
  rtl_addi(s, dest, rz, imm);
}

// rtl_mv: dest <- src1 + 0
static inline def_rtl(mv, rtlreg_t* dest, const rtlreg_t *src1) {
  if (dest != src1) rtl_add(s, dest, src1, rz);
}
// rtl_not: 
static inline def_rtl(not, rtlreg_t *dest, const rtlreg_t* src1) {
  // dest <- ~src1
  rtl_sub(s, ddest, rz, ddest);
  rtl_subi(s, ddest, ddest, 1);
}
// rtl_neg:
static inline def_rtl(neg, rtlreg_t *dest, const rtlreg_t* src1) {
  // dest <- -src1
  rtl_sub(s, dest, rz, src1);
}
// rtl_sext:
static inline def_rtl(sext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- signext(src1[(width * 8 - 1) .. 0])
  *dest = ((int)(*src1 << 8 * (4 - width)) >> 8 * (4 - width));
}
// rtl_zext:
static inline def_rtl(zext, rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- zeroext(src1[(width * 8 - 1) .. 0])
  *dest = ((unsigned)0xffffffff >> 8 * (4 - width)) & *src1;
}
// rtl_msb:
static inline def_rtl(msb, rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- src1[width * 8 - 1]
  *dest = (*src1 >> (8 * width - 1)) & 0x00000001;
}

#endif
