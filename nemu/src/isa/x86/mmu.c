#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>

int isa_vaddr_check(vaddr_t vaddr, int type, int len){
  if(cpu.cr0 & 0x80000000)
    return MEM_RET_NEED_TRANSLATE;
  else 
    return MEM_RET_OK;
}

paddr_t isa_mmu_translate(vaddr_t vaddr, int type, int len) {
  uint32_t pde_base = cpu.cr3 & ~0xfff;
  uint32_t pde_idx = (vaddr >> 22) & 0x3ff;
  uint32_t pte_idx = (vaddr >> 12) & 0x3ff;
  uint32_t pde_entry = paddr_read(pde_base + pde_idx * 4, 4);
  if(!(pde_entry & 0x1)){
    // printf("type: %d\n", type);
    // printf("pc: %x vaddr: %x pde_entry: %x\n", cpu.pc, vaddr, pde_entry);
    // printf("pde_base: %x\npde_idx: %x\n", pde_base, pde_idx);
    // printf("eax: %x ecx: %x edx: %x ebx: %x esp: %x\n", cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, cpu.esp);
  }
  assert(pde_entry & 0x1);
  uint32_t pte_base = pde_entry & ~0xfff;
  uint32_t pte_entry = paddr_read(pte_base + pte_idx * 4, 4);

  if(!(pte_entry & 0x1)){
    // printf("type: %d\n", type);
    // printf("pc: %x vaddr: %x pte_entry: %x\n", cpu.pc, vaddr, pte_entry);
    // printf("eax: %x ecx: %x edx: %x ebx: %x esp: %x\n", cpu.eax, cpu.ecx, cpu.edx, cpu.ebx, cpu.esp);
    // printf("cr3: %x\n", cpu.cr3);
  }
  assert(pte_entry & 0x1);
  return pte_entry & ~0xfff;
}


word_t vaddr_mmu_read(vaddr_t addr, int len, int type){
  paddr_t pg_base = isa_mmu_translate(addr, type, len);
  paddr_t paddr = pg_base + (addr & 0xfff);
  //assert(addr == paddr);
  assert(len == 4 || len == 1 || len == 2);

  if (addr % PAGE_SIZE <= (addr + len - 1) % PAGE_SIZE) {
    return paddr_read(paddr, len);
  } else if (addr % PAGE_SIZE > (addr + len - 1) % PAGE_SIZE) {  // crosspage
    int len2 = (addr + len - 1) % PAGE_SIZE + 1;
    int len1 = len - len2;
    paddr_t pg_base2 = isa_mmu_translate(addr + len1, type, len);

    word_t ret = 0;
    for(int i = len2 - 1;i >= 0 ;--i)
      ret = (ret << 8) + paddr_read(pg_base2 + i, 1);
    for(int i = len1 - 1;i >= 0 ;--i)
      ret = (ret << 8) + paddr_read(paddr + i, 1);
    return ret;
  }
  return 0;
}


void vaddr_mmu_write(vaddr_t addr, word_t data, int len){
  paddr_t pg_base = isa_mmu_translate(addr, MEM_TYPE_WRITE, len);
  assert(len <= 4);
  if (addr % PAGE_SIZE <= (addr + len - 1) % PAGE_SIZE) {
    paddr_t paddr = pg_base + (addr & 0xfff);
    paddr_write(paddr, data, len);
  } else if(addr % PAGE_SIZE > (addr + len - 1) % PAGE_SIZE) {
    assert(0);
  }
}

