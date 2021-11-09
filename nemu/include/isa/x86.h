#ifndef __ISA_X86_H__
#define __ISA_X86_H__

#include <common.h>


// memory
#define x86_IMAGE_START 0x100000
#define x86_PMEM_BASE 0x0

// reg

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

  /* Do NOT change the order of the GPRs' definitions. */

  /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
   * in PA2 able to directly access these registers.
   */
  
typedef struct {
  union{
    union {
      uint32_t _32;
      uint16_t _16;
      uint8_t _8[2];
    } gpr[8];
    struct{
      rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    };
  };
  vaddr_t pc;
  rtlreg_t eflags;  // eflags
  rtlreg_t cs;  // cs
  struct{
    int16_t limit;
    rtlreg_t base;
  }idtr; // idtr: Interrupt Descriptor Table Register
  struct{
    int16_t limit;
    rtlreg_t base;
  }gdtr; 
  struct{
    uint16_t selector;
    uint32_t base;  // hidden
    uint16_t limt;  // hidden
  }tr;
  rtlreg_t ss;
  rtlreg_t cr0;
  rtlreg_t cr3;
  bool INTR;
} x86_CPU_state;

// decode
typedef struct {
  bool is_operand_size_16;    // true if operand size is 16
  uint8_t ext_opcode;         // ext_code in ModR_M
  const rtlreg_t *mbase;      // point to base address
  rtlreg_t mbr;               // base address for memory
  word_t moff;                // disp
} x86_ISADecodeInfo;

#define suffix_char(width) ((width) == 4 ? 'l' : ((width) == 1 ? 'b' : ((width) == 2 ? 'w' : '?')))
//#define isa_vaddr_check(vaddr, type, len) (MEM_RET_OK)
#define x86_has_mem_exception() (false)


#endif
