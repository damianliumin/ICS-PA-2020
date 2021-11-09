void init_monitor(int, char *[]);
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  init_monitor(argc, argv);

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}


  /* In order to use the test program gen-expr, we also need to modify expr.c:
        size of tokens[] will be expanded to 65536
        switch in make_token(): checking nr_token   [2 places] 
  */
  /* Test for expr.c PA1.2 */
  /*#include<stdio.h>
  #include<string.h>
  #include<assert.h>
  #include<common.h>
  #include<stdlib.h>
  #include"/home/daniel/ics2020/nemu/src/monitor/debug/expr.h"

  char save[65536 + 20] = {0};

  // In main() 
  FILE *fp = fopen("/home/daniel/ics2020/nemu/tools/gen-expr/input", "r");
  if(fp == NULL){
    printf("Fail of open a file\n");
  }

  int total = 0;
  int err = 0;
  while(fgets(save, sizeof(save), fp) != NULL){
    ++ total;

    uint32_t ans = (uint32_t)atol(save);
    
    char* e = strchr(save, ' ');
    assert(e != NULL);
    e = e + 1;
    e[strlen(e) - 1] = '\0';
    
    bool success = true;
    unsigned ret = expr(e, &success);

    if(success == 0){
      ++ err;
      printf("No.%d: cannot parse expression\n", total);
      //printf("%s\n", e);
    } else if(ret != ans){
      ++ err;
      printf("No.%d: wrong answer -- my:%u ans:%u\n", total, ret, ans);
      printf("%s\n", e);
    }
  }  
  if(feof(fp))
    printf("End of file reached\n");
  printf("Total: %d  Errors: %d\n", total, err);
  */
