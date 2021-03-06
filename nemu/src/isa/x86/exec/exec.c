#include <cpu/exec.h>
#include "../local-include/decode.h"
#include "all-instr.h"

/* width of opcodes: 8, 16 or 32 */
/* parameter width 1: set operand width to 8 (1 byte)
             width 0: set operand width according to is_operand_size_16  */
static inline void set_width(DecodeExecState *s, int width) {
  if (width == -1) return;
  if (width == 0) {
    width = s->isa.is_operand_size_16 ? 2 : 4;
  }
  s->src1.width = s->dest.width = s->src2.width = width;
}

/* 0x80 (w = 1), 0x81 (w = 2,4), 0x83 (w = 2,4, wi = 1)*/
static inline def_EHelper(gp1) {
  switch (s->isa.ext_opcode) {
    EXW (0, add, -1) 
    EXW (1, or,  -1) 
    EXW (2, adc, -1) 
    EXW (3, sbb, -1)
    EXW (4, and, -1) 
    EXW (5, sub, -1) 
    EXW (6, xor, -1)
    EXW (7, cmp, -1)
  }
}

// be careful with cl2E, it will use EAX rather than CL
/* 0xc0(w=1), 0xc1(2,4), 0xd0(1), 0xd1(2,4), 0xd2(1), 0xd3(2,4) */
static inline def_EHelper(gp2) {
  switch (s->isa.ext_opcode) {
    EXW  (0, rol, -1) 
    EMPTY(1) EMPTY(2) EMPTY(3)
    EXW  (4, shl, -1)
    EXW  (5, shr, -1) 
    EMPTY(6) 
    EXW  (7, sar, -1)
  }
}

/* 0xf6 (w=1), 0xf7 (w=2,4) (decode = E) */
static inline def_EHelper(gp3) {
  switch (s->isa.ext_opcode) {
    IDEXW(0, test_I, test, -1) 
    EMPTY(1) 
    EXW  (2, not, -1) 
    EXW  (3, neg, -1)
    EXW  (4, mul, -1) 
    EXW  (5, imul1, -1) 
    EXW  (6, div, -1) 
    EXW  (7, idiv, -1)
  }
}

/* 0xfe */
static inline def_EHelper(gp4) {
  switch (s->isa.ext_opcode) {
    EXW  (0, inc, -1)
    EXW  (1, dec, -1) 
    EMPTY(2) EMPTY(3)
    EMPTY(4) EMPTY(5) EMPTY(6) EMPTY(7)
  }
}

/* 0xff (decode = E) */
static inline def_EHelper(gp5) {
  switch (s->isa.ext_opcode) {
    EXW  (0, inc, -1) 
    EXW  (1, dec, -1)
    EXW  (2, call_rm, -1) 
    EMPTY(3)
    EXW  (4, jmp_rm, -1) 
    EMPTY(5) 
    EXW  (6, push, -1) 
    EMPTY(7)
  }
}

/* 0x0f 0x01*/
static inline def_EHelper(gp6) {
  switch (s->isa.ext_opcode) {
    EMPTY(0) EMPTY(1) EMPTY(2) 
    EXW  (3, ltr, -1)
    EMPTY(4) EMPTY(5) EMPTY(6) EMPTY(7)
  }
}

/* 0x0f 0x01*/
static inline def_EHelper(gp7) {
  switch (s->isa.ext_opcode) {
    EMPTY(0) EMPTY(1) 
    EXW  (2, lgdt, -1) 
    EXW  (3, lidt, -1)
    EMPTY(4) EMPTY(5) EMPTY(6) EMPTY(7)
  }
}

/* opcodes that starts with 0x0f, read another byte */
static inline def_EHelper(2byte_esc) {
  uint8_t opcode = instr_fetch(&s->seq_pc, 1);
  s->opcode = opcode;
  switch (opcode) {
  /* TODO: Add more instructions!!! */
    IDEXW(0x00, E, gp6, 2)   // only for ltr !
    IDEX (0x01, gp7_E, gp7)
    IDEXW(0x20, mov_C2E, mov_cr2r, -1)
    IDEXW(0x22, mov_E2C, mov_r2cr, -1)
    IDEXW(0x80, J, jcc, -1)
    IDEXW(0x81, J, jcc, -1)
    IDEXW(0x82, J, jcc, -1)
    IDEXW(0x83, J, jcc, -1)
    IDEXW(0x84, J, jcc, -1)
    IDEXW(0x85, J, jcc, -1)
    IDEXW(0x86, J, jcc, -1)
    IDEXW(0x87, J, jcc, -1)
    IDEXW(0x88, J, jcc, -1)
    IDEXW(0x89, J, jcc, -1)
    IDEXW(0x8a, J, jcc, -1)
    IDEXW(0x8b, J, jcc, -1)
    IDEXW(0x8c, J, jcc, -1)
    IDEXW(0x8d, J, jcc, -1)
    IDEXW(0x8e, J, jcc, -1)
    IDEXW(0x8f, J, jcc, -1)
    IDEXW (0x90, E, setcc, 1)
    IDEXW (0x91, E, setcc, 1)
    IDEXW (0x92, E, setcc, 1)
    IDEXW (0x93, E, setcc, 1)
    IDEXW (0x94, E, setcc, 1)
    IDEXW (0x95, E, setcc, 1)
    IDEXW (0x96, E, setcc, 1)
    IDEXW (0x97, E, setcc, 1)
    IDEXW (0x98, E, setcc, 1)
    IDEXW (0x99, E, setcc, 1)
    IDEXW (0x9a, E, setcc, 1)
    IDEXW (0x9b, E, setcc, 1)
    IDEXW (0x9c, E, setcc, 1)
    IDEXW (0x9d, E, setcc, 1)
    IDEXW (0x9e, E, setcc, 1)
    IDEXW (0x9f, E, setcc, 1)
    IDEXW (0xac, Ib_G2E, shrd, -1)
    IDEXW (0xaf, E2G, imul2, -1)
    IDEXW(0xb6, mov_E2G, movzx, 1)
    IDEXW(0xb7, mov_E2G, movzx, 2)
    IDEXW(0xbe, mov_E2G, movsx, 1)
    IDEXW(0xbf, mov_E2G, movsx, 2)
    default: exec_inv(s);
  }
}

/* fetch, decode and execute */
static inline void fetch_decode_exec(DecodeExecState *s) {
  uint8_t opcode;
again:
  opcode = instr_fetch(&s->seq_pc, 1);  // fetch the opcode(1 byte)
  s->opcode = opcode;
  // example:
  // case 0xb0: set_width(s, 1); decode_mov_I2r(s); exec_mov(s); break;
  switch (opcode) {
    IDEXW(0x00, G2E, add, 1)
    IDEX (0x01, G2E, add)
    IDEXW(0x02, E2G, add, 1)
    IDEX (0x03, E2G, add)
    IDEXW(0x04, I2a, add, 1)
    IDEX (0x05, I2a, add)
    IDEXW(0x08, G2E, or, 1)
    IDEX (0x09, G2E, or)
    IDEXW(0x0a, E2G, or, 1)
    IDEX (0x0b, E2G, or)
    IDEXW(0x0c, I2a, or, 1)
    IDEX (0x0d, I2a, or)
    EX   (0x0f, 2byte_esc)   // read one more byte for opcode
    IDEXW(0x10, G2E, adc, 1)
    IDEX (0X11, G2E, adc)
    IDEXW(0x12, E2G, adc, 1)
    IDEX (0x13, E2G, adc)
    IDEXW(0x14, I2a, adc, 1)
    IDEX (0x15, I2a, adc)
    IDEXW(0x18, G2E, sbb, 1)
    IDEX (0x19, G2E, sbb)
    IDEXW(0x1a, E2G, sbb, 1)
    IDEX (0x1b, E2G, sbb)
    IDEXW(0x1c, I2a, sbb, 1)
    IDEX (0x1d, I2a, sbb)
    IDEXW(0x20, G2E, and, 1)
    IDEX (0x21, G2E, and)
    IDEXW(0x22, E2G, and, 1)
    IDEX (0x23, E2G, and)
    IDEXW(0x24, I2a, and, 1)
    IDEX (0x25, I2a, and)
    IDEXW(0x28, G2E, sub, 1)
    IDEX (0x29, G2E, sub)
    IDEXW(0x2a, E2G, sub, 1)
    IDEX (0x2b, E2G, sub)
    IDEXW(0x2c, I2a, sub, 1)
    IDEX (0x2d, I2a, sub)
    IDEXW(0x30, G2E, xor, 1)
    IDEX (0x31, G2E, xor)
    IDEXW(0x32, E2G, xor, 1)
    IDEX (0x33, E2G, xor)
    IDEXW(0x34, I2a, xor, 1)
    IDEX (0x35, I2a, xor)
    IDEXW(0x38, G2E, cmp, 1)
    IDEX (0x39, G2E, cmp)
    IDEXW(0x3a, E2G, cmp, 1)
    IDEX (0x3b, E2G, cmp)
    IDEXW(0x3c, I2a, cmp, 1)
    IDEX (0x3d, I2a, cmp)
    IDEX (0x40, r, inc)
    IDEX (0x41, r, inc)
    IDEX (0x42, r, inc)
    IDEX (0x43, r, inc)
    IDEX (0x44, r, inc)
    IDEX (0x45, r, inc)
    IDEX (0x46, r, inc)
    IDEX (0x47, r, inc)
    IDEX (0x48, r, dec)
    IDEX (0x49, r, dec)
    IDEX (0x4a, r, dec)
    IDEX (0x4b, r, dec)
    IDEX (0x4c, r, dec)
    IDEX (0x4d, r, dec)
    IDEX (0x4e, r, dec)
    IDEX (0x4f, r, dec)
    IDEX (0x50, r, push)
    IDEX (0x51, r, push)
    IDEX (0x52, r, push)
    IDEX (0x53, r, push)
    IDEX (0x54, r, push)
    IDEX (0x55, r, push)
    IDEX (0x56, r, push)
    IDEX (0x57, r, push)
    IDEX (0x58, r, pop)
    IDEX (0x59, r, pop)
    IDEX (0x5a, r, pop)
    IDEX (0x5b, r, pop)
    IDEX (0x5c, r, pop)
    IDEX (0x5d, r, pop)
    IDEX (0x5e, r, pop)
    IDEX (0x5f, r, pop)
    EX   (0x60, pusha)
    EX   (0x61, popa)
    IDEXW(0x6a, I, push, 1)
    IDEX (0x68, I, push)
    IDEX (0x69, SI_E2G, imul3)
    IDEXW(0x70, J, jcc, 1)
    IDEXW(0x71, J, jcc, 1)
    IDEXW(0x72, J, jcc, 1)
    IDEXW(0x73, J, jcc, 1)
    IDEXW(0x74, J, jcc, 1)
    IDEXW(0x75, J, jcc, 1)
    IDEXW(0x76, J, jcc, 1)
    IDEXW(0x77, J, jcc, 1)
    IDEXW(0x78, J, jcc, 1)
    IDEXW(0x79, J, jcc, 1)
    IDEXW(0x7a, J, jcc, 1)
    IDEXW(0x7b, J, jcc, 1)
    IDEXW(0x7c, J, jcc, 1)
    IDEXW(0x7d, J, jcc, 1)
    IDEXW(0x7e, J, jcc, 1)
    IDEXW(0x7f, J, jcc, 1)
    IDEXW(0x80, I2E, gp1, 1)
    IDEX (0x81, I2E, gp1)
    IDEX (0x83, SI2E, gp1)
    IDEXW(0x84, G2E, test, 1)
    IDEX (0x85, G2E, test)
    IDEXW(0x88, mov_G2E, mov, 1)
    IDEX (0x89, mov_G2E, mov)
    IDEXW(0x8a, mov_E2G, mov, 1)
    IDEX (0x8b, mov_E2G, mov)
    IDEX (0x8d, lea_M2G, lea)
    IDEX (0x8f, E, push)
    EX   (0x90, nop)
    IDEX (0x91, r, xchg)
    IDEX (0x92, r, xchg)
    IDEX (0x93, r, xchg)
    IDEX (0x94, r, xchg)
    IDEX (0x95, r, xchg)
    IDEX (0x96, r, xchg)
    IDEX (0x97, r, xchg)
    EX   (0x98, cwtl)
    EX   (0x99, cltd)
    IDEXW(0xa0, O2a, mov, 1)
    IDEX (0xa1, O2a, mov)
    IDEXW(0xa2, a2O, mov, 1)
    IDEX (0xa3, a2O, mov)
    EXW  (0xa4, movs, 1)
    EX   (0xa5, movs)
    IDEXW(0xa8, I2a, test, 1)
    IDEX (0xa9, I2a, test)
    IDEXW(0xb0, mov_I2r, mov, 1)
    IDEXW(0xb1, mov_I2r, mov, 1)
    IDEXW(0xb2, mov_I2r, mov, 1)
    IDEXW(0xb3, mov_I2r, mov, 1)
    IDEXW(0xb4, mov_I2r, mov, 1)
    IDEXW(0xb5, mov_I2r, mov, 1)
    IDEXW(0xb6, mov_I2r, mov, 1)
    IDEXW(0xb7, mov_I2r, mov, 1)
    IDEX (0xb8, mov_I2r, mov)
    IDEX (0xb9, mov_I2r, mov)
    IDEX (0xba, mov_I2r, mov)
    IDEX (0xbb, mov_I2r, mov)
    IDEX (0xbc, mov_I2r, mov)
    IDEX (0xbd, mov_I2r, mov)
    IDEX (0xbe, mov_I2r, mov)
    IDEX (0xbf, mov_I2r, mov)
    IDEXW(0xc0, gp2_Ib2E, gp2, 1)
    IDEX (0xc1, gp2_Ib2E, gp2)
    IDEXW(0xc2, I, ret_imm, 2)
    EX   (0xc3, ret)
    IDEXW(0xc6, mov_I2E, mov, 1)
    IDEX (0xc7, mov_I2E, mov)
    EX   (0xc9, leave)
    IDEXW(0xcd, I, int, 1)
    EX   (0xcf, iret)
    IDEXW(0xd0, gp2_1_E, gp2, 1)
    IDEX (0xd1, gp2_1_E, gp2)
    IDEXW(0xd2, gp2_cl2E, gp2, 1)
    IDEX (0xd3, gp2_cl2E, gp2)
    EX   (0xd6, nemu_trap)
    IDEXW(0xe4, in_I2a, in, 1)
    IDEX (0xe5, in_I2a, in)
    IDEXW(0xe6, out_a2I, out, 1)
    IDEX (0xe7, out_a2I, out)
    IDEX (0xe8, J, call) // call
    IDEX (0xe9, J, jmp)
    IDEXW(0xeb, J, jmp, 1)
    IDEXW(0xec, in_dx2a, in, 1)
    IDEX (0xed, in_dx2a, in)
    IDEXW(0xee, out_a2dx, out, 1)
    IDEX (0xef, out_a2dx, out)
    IDEXW(0xf6, E, gp3, 1)
    IDEX (0xf7, E, gp3)
    IDEXW(0xfe, E, gp4, 1)
    IDEX (0xff, E, gp5)
  case 0x66: s->isa.is_operand_size_16 = true; goto again;  // operand-size-prefix: 16bit
  default: exec_inv(s);
  }
}

void query_intr(DecodeExecState *s);

vaddr_t isa_exec_once() {
  DecodeExecState s;
  s.is_jmp = 0;
  s.isa = (ISADecodeInfo) { 0 };
  s.seq_pc = cpu.pc;

  fetch_decode_exec(&s);
  update_pc(&s);

  query_intr(&s);
  return s.seq_pc;
}
