#include <isa.h>
#include <monitor/difftest.h>
#include "../local-include/reg.h"
#include "difftest.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  for(int i = 0 ;i < 9 ;++i)
    if(*((rtlreg_t*)ref_r + i) != *((rtlreg_t*)&cpu + i))
      return false;
  //cpu.eflags = ref_r->eflags;
  return true;
}

void isa_difftest_attach() {
}
