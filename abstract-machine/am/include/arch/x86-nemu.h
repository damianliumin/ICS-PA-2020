#ifndef ARCH_H__
#define ARCH_H__
#include <stdint.h>

struct Context {
  void *cr3; // push $0
  uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha
  int irq; // push id
  uintptr_t eip, cs, eflags; // int
  uintptr_t esp3, ss3; // int
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif
