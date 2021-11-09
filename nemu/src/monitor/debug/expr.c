#include <isa.h>
#include "../../../include/memory/paddr.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ, TK_UEQ, TK_LEQ, TK_L, TK_GEQ, TK_G, 
  TK_AND, TK_OR, 
  TK_REGISTER, TK_NUMBER,

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"-", '-'},           // minus or negative
  {"\\*", '*'},         // multiply or access memory
  {"\\/", '/'},         // divide
  {"\\$[a-zA-Z]{2,6}", TK_REGISTER},  // register
  {"(0x)?[0-9a-fA-F]+u?", TK_NUMBER},        // number(deximal or hex), u means unsigned for test
  {"\\(", '('},         // left-bracket
  {"\\)", ')'},         // right-bracket
  {"==", TK_EQ},        // equal
  {"\\!=", TK_UEQ},     // unequal
  {"<=", TK_LEQ},       // less or equal
  {"<", TK_L},          // less
  {">=", TK_GEQ},       // greater or equal
  {">", TK_G},          // Greater
  {"&&", TK_AND},       // logic and
  {"\\|\\|", TK_OR}     // logic or
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;


/* size 65536 is designed for test */
//static Token tokens[65536] __attribute__((used)) = {};
static Token tokens[320] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        
        switch (rules[i].token_type) {
          case '+': case '-': case '*': case '/': case '(': case ')': 
          case TK_EQ: case TK_UEQ: case TK_LEQ: case TK_L: case TK_GEQ: case TK_G:
          case TK_AND: case TK_OR: 
            if(nr_token > 319)
              return false;
            //if(nr_token > 65535)
              //return false;

            tokens[nr_token].type = rules[i].token_type;
            ++ nr_token;
            break;

          case TK_REGISTER: case TK_NUMBER:
            if(nr_token > 319)
              return false;
            //if(nr_token > 65535)
              //return false;

            tokens[nr_token].type = rules[i].token_type;
            if(substr_len > 31)
              return false;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            (tokens[nr_token].str)[substr_len] = '\0';
            ++ nr_token;
            break;

          case TK_NOTYPE:
            break;

          default:
            return false;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int bad_er;

static uint32_t hex_to_int(char* str){
  if(strlen(str)-2 > 8){
    bad_er = 1;
    return 0;
  }

  int t = strlen(str) - 1;
 
  /* for test */
  if(str[strlen(str) - 1] == 'u')
    -- t;

  uint32_t base = 1;
  uint32_t val = 0;

  for(int i = t ;i > 1 ;-- i){
    if('0' <= str[i] && str[i] <= '9')
      val += base * (str[i] - '0');
    else if('a' <= str[i] && str[i] <= 'f')
      val += base * (str[i] - 'a' + 10);
    else if('A' <= str[i] && str[i] <= 'F')
      val += base * (str[i] - 'A' + 10);
    else{
      bad_er = 1;
      return 0;
    }
    
    base *= 16;
  }
  return val;
}

/* fetch number from a register, hex or decimal */
static uint32_t fetch_number(int id){
  switch (tokens[id].type) {
      case TK_REGISTER:{
        bool success = true;
        uint32_t ret = isa_reg_str2val(tokens[id].str, &success);
        if(success == true)
          return ret;
        else{
          bad_er = 1;
          return 0;
        }  
      }
      case TK_NUMBER:
        if(strlen(tokens[id].str) > 1 && (tokens[id].str)[1] == 'x')
          return hex_to_int(tokens[id].str);
        else
          return (uint32_t)atol(tokens[id].str);

      default:
        bad_er = 1;
        return 0;
    }
}

/* check whether the expression is enclosed in brackets */
static int check_parentheses(int p, int q){
  if(tokens[p].type != '(' || tokens[q].type != ')')
    return 0;

  if(p + 1 == q){
    bad_er = 1;
    return 0;
  }
  
  int lb_num = 0;

  for(int i = p + 1 ;i < q ;++ i){
    if(tokens[i].type == '(')
      ++lb_num;
    else if(tokens[i].type == ')' && lb_num == 0)
      return 0;
    else if(tokens[i].type == ')' && lb_num > 0)
      --lb_num;
  }
  return 1;
}

/* return id of the main-operator */
static int main_operator(int p, int q){
  int mul_div = -1, sum_sub = -1, gel = -1, eq_ueq = -1, and = -1, or = -1;
  int lb_num = 0;
  for(int i = p; i <= q ;++ i){
    if(tokens[i].type == '(')
      ++ lb_num;
    if(tokens[i].type == ')'){
      if(lb_num <= 0){
        bad_er = 1;
        return 0;
      }
      -- lb_num;
    }

    if(lb_num != 0)
      continue;

    switch(tokens[i].type){
      case TK_OR:
        or = i; break;
      case TK_AND:
        and = i; break;
      case TK_EQ: case TK_UEQ:
        eq_ueq = i; break;
      case TK_LEQ: case TK_L: case TK_GEQ: case TK_G:
        gel = i; break;
      case '+': case '-':
        /* warning: sum_sub will still record unary operation if the previous operator has
        lower priority */
        if(i > p && tokens[i-1].type != '-' && tokens[i-1].type != '+'
          && tokens[i-1].type != '*' && tokens[i-1].type != '/')
          sum_sub = i;
        break;
      case '*': case '/':
      /* warning: mul_div will still record unary operation if the previous operator has
        lower priority */
        if(i > p && tokens[i-1].type != '*' && tokens[i-1].type != '/'
          && tokens[i-1].type != '-' && tokens[i-1].type != '+')
          mul_div = i;
        break;
      /* '(' ')' in default */
      default:
        break;
    }
  }
  if(or != -1)
    return or;
  else if(and != -1)
    return and;
  else if(eq_ueq != -1)
    return eq_ueq;
  else if(gel != -1)
    return gel;
  else if(sum_sub != -1)
    return sum_sub;
  else if(mul_div != -1){
    /* deal with "-*(0x100000)"" */
    if(mul_div == p + 1 && tokens[p].type == '-') return p;
    return mul_div;
  }
  else{
    if(tokens[p].type == '*' || tokens[p].type == '-')
      return p;
    else{
      bad_er = 1;
      return 0;
    }
  }
}

/* calculate the value from the tokens recursively */
static uint32_t eval(int p, int q){
  if(p > q){
    bad_er = 1;
    return 0;
  } 
  else if (p == q) {
    /* fetch the value, assert if it is not a value */
    uint32_t num = fetch_number(p);
    if(bad_er == 1)
      return 0;
    return num;
  } 
  else if (check_parentheses(p, q) == 1){
    /* remove the brackets, assert if brackets enclose nothing */
    if(bad_er == 1)
      return 0;
    return eval(p+1, q-1);
  }
  else {
    int op = main_operator(p, q);

    if(bad_er == 1) return 0;

    uint32_t val1 = 0, val2 = 0;
    if(!(op == p && ((tokens[p].type == '*') || (tokens[p].type == '-')))){
      val1 = eval(p, op-1);
      if(bad_er == 1) return 0;
    }
    val2 = eval(op+1, q);
    if(bad_er == 1) return 0;

    switch(tokens[op].type){
      case '+':
        return val1 + val2;
      case '-':
        if(op == p){
          return -val2;
        } else {
          return val1 - val2; 
        }
      case '/':
        if(val2 == 0){
          /* test */
          //printf("\nval2 = 0! \n");
          //printf("tokens:%d p:%d q:%d\n", nr_token, p, q);
          bad_er = 1;
          return 0;
        }
        return val1 / val2;
      case '*':
        if(op == p){
          assert(val2 + 3 < PMEM_SIZE);
          uint32_t *mem32 = (uint32_t*)guest_to_host((paddr_t)val2);
          return *mem32;
        } else {
          return val1 * val2;
        }
      case TK_OR:
        return val1 || val2;
      case TK_AND:
        return val1 && val2;
      case TK_EQ:
        return val1 == val2;
      case TK_UEQ:
        return val1 != val2;
      case TK_LEQ:
        return val1 <= val2;
      case TK_L:
        return val1 < val2;
      case TK_GEQ:
        return val1 >= val2;
      case TK_G:
        return val1 > val2;
      default:
        bad_er = 1;
        return 0;
    }

  }
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  bad_er = 0;
  uint32_t ret = eval(0, nr_token-1);
  if(bad_er){
    *success = false;
    return 0;
  }
  *success = true;
  return (word_t)ret;
}
