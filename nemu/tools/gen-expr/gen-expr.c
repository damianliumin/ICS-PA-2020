#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
#define SIZE 65500
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = (unsigned)(%s); "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

/* designed for buf */
static int pos;

static inline void gen_space(){
  if(pos >= 65500) return;

  int t = rand() % 8;
  switch(t){
    case 0:
      buf[pos++] = ' ';
      break;
    case 1:
      gen_space();
      break;
    default:
      return;
  }
}

static inline void gen_num(){
  if(pos >= SIZE) return;

  /* test unary operator '-' */
  if(rand()%2 == 1)
    buf[pos++] = '-';

  uint32_t num = (uint32_t)rand();
  char temp[20] = {0};
  if(num == 0)
    buf[pos++] = '0';
  int cnt = 0;
  while(num != 0){
    temp[++cnt] = num % 10 + '0';
    num /= 10;
  }
  for(int i = cnt; i >= 1 ;-- i){
    buf[pos++] = temp[i];
  }
  /* If we do not use unsigned, the integers will be considered as
  signed, and the answers will differ from expected results(unsigned) a bit. (due to division)
  Besides, signed int cannot pass the compilation when overflow occurs.
   */
  buf[pos++] = 'u';
}

static inline void gen_op(){
  if(pos >= SIZE) return;

  int op = rand() % 4;
  switch(op){
    case 0:
      buf[pos++] = '+'; break;
    case 1:
      buf[pos++] = '-'; break;
    case 2:
      buf[pos++] = '*'; break;
    default:
      buf[pos++] = '/'; break;
  }
}

static inline void gen_rand_expr() {
  if(pos >= SIZE) return;
  int choose = rand() % 30;
  if(0 <= choose && choose <= 10){
    gen_num();
    gen_space();
  } else if(11 <= choose && choose <= 19) {
    if(rand()%3==0) buf[pos++] = '-';
    buf[pos++] = '(';
    gen_space();
    gen_rand_expr();
    if(pos >= 65500) return;
    buf[pos++] = ')';
    gen_space();
  } else {
    gen_rand_expr();
    gen_op();
    gen_space();
    gen_rand_expr();
  }
  buf[pos] = '\0';
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;

  for (i = 0; i < loop; i ++) {
    pos = 0;
    gen_rand_expr();

    if(pos >= SIZE){
      -- i;
      continue;
    }

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);
    
    /* add -Werror in case that unsigned long calculation will dismiss divide-by-0 warnings */
    /* Q: haven't figured out why the total examples is less than loop even after adding --i */
    int ret = system("gcc -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    char result_num[20] = {0};
    unsigned result;
    if(fscanf(fp, "%s", result_num));
    if(result_num[0] == '\0'){
      /* handle floating point exception */
      pclose(fp);
      -- i;
    }else{
      result = (unsigned)atol(result_num);
      pclose(fp);
      printf("%u %s\n", result, buf);
    }
  }
  return 0;
}
