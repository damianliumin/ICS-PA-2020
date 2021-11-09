#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;

  while (1) {
    if(j % 100 == 1){
      Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    }
    j ++;
    yield();
  }
}

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*entry)(void *), void* arg);
void context_uload(PCB *, const char *, char *const [], char *const []);

void init_proc() {

  context_kload(&pcb[0], hello_fun, (void*)0x7);

  // context_kload(&pcb[1], hello_fun, (void*)0x2);

  char *param1[10] = {"/bin/nslider", NULL};
  context_uload(&pcb[1], "/bin/nslider", param1, NULL);

  char *param2[10] = {"/bin/pal", "--skip", NULL};
  context_uload(&pcb[2], "/bin/pal", param2, NULL);

  char *param3[10] = {"/bin/bird", NULL};
  context_uload(&pcb[3], "/bin/bird", param3, NULL);
  
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/nterm");
}

int schedule_usr_current = 1;

Context* schedule(Context *prev) {
  current->cp = prev;

  // current = &pcb[0];
  
  // if(current != &pcb[0])
  //   current = &pcb[0];
  // else 
  //   current = &pcb[1];
  
  if(current != &pcb[schedule_usr_current]){ // kernel
    current = &pcb[schedule_usr_current];
  } else {  // execute user program first, or kernel functions cr3 will not be NULL
    // user
    current = &pcb[0];
  }
  return current->cp;
}

void context_kload(PCB *pcb, void (*entry)(void *), void* arg){
  Area kstack;
  kstack.start = (void*)pcb;
  kstack.end = (void*)((char*)pcb + sizeof(PCB));
  pcb->cp = kcontext(kstack, entry, arg);
}

