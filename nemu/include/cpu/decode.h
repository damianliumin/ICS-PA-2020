#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include <isa.h>

#define OP_STR_SIZE 40
enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

typedef struct {
  uint32_t type;    // type of operand
  int width;        // width of operand
  union {
    uint32_t reg;   // reg
    word_t imm;     // unsigned imm
    sword_t simm;   // signed imm
  };
  rtlreg_t *preg;   // pointer to an existing register/ val
  rtlreg_t val;     // store the val if reg is not 32bit
  char str[OP_STR_SIZE];
} Operand;

typedef struct {
  uint32_t opcode;  // opcode
  vaddr_t seq_pc;   // sequential pc
  uint32_t is_jmp;  // tell whether to jump
  vaddr_t jmp_pc;   // jump pc
  Operand src1, dest, src2;  // source and dest operands
  int width;        // width of the opcodes: 8, 16 or 32
  rtlreg_t tmp_reg[4]; // temporary register
  ISADecodeInfo isa;
} DecodeExecState;

#define def_DHelper(name) void concat(decode_, name) (DecodeExecState *s)

#ifdef DEBUG
#define print_Dop(...) snprintf(__VA_ARGS__)
#else
#define print_Dop(...)
#endif

#endif
