#ifndef __C_OP_H__
#define __C_OP_H__

#include <common.h>

/* This header contains definition of basic operations for rtl */

#define c_shift_mask 0x1f

#define c_add(a, b) ((a) + (b))
#define c_sub(a, b) ((a) - (b))
#define c_and(a, b) ((a) & (b))
#define c_or(a, b)  ((a) | (b))
#define c_xor(a, b) ((a) ^ (b))
#define c_shl(a, b) ((a) << ((b) & c_shift_mask))
#define c_shr(a, b) ((a) >> ((b) & c_shift_mask))
#define c_sar(a, b) ((sword_t)(a) >> ((b) & c_shift_mask))


#define c_mul_lo(a, b) ((a) * (b)) // unsigned mul low 32 bits
#define c_imul_lo(a, b) ((sword_t)(a) * (sword_t)(b)) // signed mul low 32 bits
#define c_mul_hi(a, b) (((uint64_t)(a) * (uint64_t)(b)) >> 32) // unsigned mul high 32 bits
#define c_imul_hi(a, b) (((int64_t)(sword_t)(a) * (int64_t)(sword_t)(b)) >> 32) // signed mul high 32 bits

#define c_div_q(a, b) ((a) / (b)) // quotient of unsigned
#define c_div_r(a, b)  ((a) % (b)) // remain of unsigned
#define c_idiv_q(a, b) ((sword_t)(a) / (sword_t)(b)) // quotient of signed
#define c_idiv_r(a, b)  ((sword_t)(a) % (sword_t)(b)) // remain of signed

// relation operation
static inline bool interpret_relop(uint32_t relop, const rtlreg_t src1, const rtlreg_t src2) {
  switch (relop) {
    case RELOP_FALSE: return false;
    case RELOP_TRUE: return true;
    case RELOP_EQ: return src1 == src2;
    case RELOP_NE: return src1 != src2;
    case RELOP_LT: return (sword_t)src1 <  (sword_t)src2;
    case RELOP_LE: return (sword_t)src1 <= (sword_t)src2;
    case RELOP_GT: return (sword_t)src1 >  (sword_t)src2;
    case RELOP_GE: return (sword_t)src1 >= (sword_t)src2;
    case RELOP_LTU: return src1 < src2;
    case RELOP_LEU: return src1 <= src2;
    case RELOP_GTU: return src1 > src2;
    case RELOP_GEU: return src1 >= src2;
    default: panic("unsupport relop = %d", relop);
  }
}

#endif
