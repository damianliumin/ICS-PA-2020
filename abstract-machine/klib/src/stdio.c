#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// for output format
enum {FMT_MINUS, FMT_PLUS, FMT_SPACE, FMT_POUND, FMT_ZERO, 
  FMT_LL, FMT_L, FMT_H, FMT_HH, FMT_SIGNED, FMT_UPPERCASE };
enum {TYPE_FLOAT, TYPE_INTEGER, TYPE_CHAR, TYPE_STR };

static char* stophere;
static int total_ch;
static int std_out;

typedef struct{
  int flags;
  int width;
  int precision;
  int base;
} fmt_info;

static void pchar(char *buf, char c){
  if(c != '\0')
    ++total_ch;
  if(std_out){
      if(c != '\0') putch(c);
  }else{
    if(stophere == NULL || buf < stophere)
      *buf = c;
    else
      *stophere = '\0';
  }
}

static size_t strlen_p(const char *s) {
  size_t count = 0;
  while(*s != '\0'){
    count ++;
    s ++;
  }
  return count;
}

static char *strcpy_p(char* dst,const char* src) {
  char *save = dst;
  while(*src != '\0')
    pchar(dst++, *(src++));
  pchar(dst, *src);   // save '\0'
  return save;
}

static int format_processor(fmt_info *s, const char *fmt, va_list *ap){
  while(1){
    switch(*fmt){
      case '-': s->flags |= (1u << FMT_MINUS); break;
      case '+': s->flags |= (1u << FMT_PLUS); break;
      case ' ': s->flags |= (1u << FMT_SPACE); break;
      case '#': s->flags |= (1u << FMT_POUND); break;
      case '0': s->flags |= (1u << FMT_ZERO); break;
      case '*': s->width = va_arg(*ap, int); break;
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':{
        s->width = atoi(fmt);
        while(*(fmt+1) <= '9' && *(fmt+1) >= '0')
          fmt ++;
        break;
      }
      case '.':{ assert(0); break; }
      case 'l':{
        if(*(fmt+1) == 'l'){
          s->flags |= (1u << FMT_LL);  fmt++;
        } else 
          s->flags |= (1u << FMT_L);
        break;
      }
      case 'h':{
        if(*(fmt+1) == 'h'){
          s->flags |= (1u << FMT_HH);  fmt++;
        } else 
          s->flags |= (1u << FMT_H);
        break;
      }
      case 's': return TYPE_STR;
      case 'c': return TYPE_CHAR;
      case 'd': case 'i': case 'u': case 'o': case 'x': case 'X':{ 
        if(*fmt == 'X')
          s->flags |= (1u << FMT_UPPERCASE);
        if(*fmt == 'd' || *fmt == 'i')
          s->flags |= (1u << FMT_SIGNED);
        if(*fmt == 'o') s->base = 8;
        else if(*fmt == 'x' || *fmt == 'X') s->base = 16;
        else s->base = 10;
        return TYPE_INTEGER;
      }
      case 'p':{
        s->flags |= (1u << FMT_POUND); // 0x
        s->base = 16; // hex
        return TYPE_INTEGER;
      }
      case 'f': case 'F': case 'a': case 'A': 
      case 'e': case 'E': case 'g': case 'G': {
        if(*fmt >= 'A' && *fmt <= 'Z')
          s->flags |= (1u << FMT_UPPERCASE);
        assert(0);
        return TYPE_FLOAT;
      }
      default: assert(0);
    }
    fmt ++;
  }
}

static void itoa(fmt_info *s, char *str, va_list *ap){
  long long val = 0;
  int sign = 0;
  if((s->flags >> FMT_HH) & 1u) val = (signed char)va_arg(*ap, int);
  else if((s->flags >> FMT_H) & 1u) val = (short)va_arg(*ap, int);
  else if((s->flags >> FMT_L) & 1u) val = (long)va_arg(*ap, long);
  else if((s->flags >> FMT_LL) & 1u) val = va_arg(*ap, long long);
  else val = (int)va_arg(*ap, int);
  
  if(val == 0x8000000000000000){  //special case
    strcpy(str, "8085774586302733229-");
    return;
  }
  if(val == 0){  // special case
    *str = '0';
    return;
  }
  if(val < 0) {
    sign = 1;
    val = -val;
  }
  while(val != 0){
    *(str++) = val % 10 + '0';
    val /= 10;
  }
  if(sign) *(str++) = '-';
  else if((s->flags >> FMT_PLUS) & 1u) *(str++) = '+';
  else if((s->flags >> FMT_SPACE) & 1u) *(str++) = ' ';
  *str = '\0';
}

static void utoa(fmt_info *s, char *str, va_list *ap){
  char num[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
  if((s->flags >> FMT_UPPERCASE) & 1u)
    for(int i = 10 ;i <= 15 ;++i)
      num[i] -= 'a' - 'A';
  unsigned long long val = 0;
  if((s->flags >> FMT_HH) & 1u) val = (unsigned char)va_arg(*ap, unsigned int);
  else if((s->flags >> FMT_H) & 1u) val = (unsigned short)va_arg(*ap, unsigned int);
  else if((s->flags >> FMT_L) & 1u) val = (unsigned long)va_arg(*ap, unsigned long);
  else if((s->flags >> FMT_LL) & 1u) val = va_arg(*ap, unsigned long long);
  else val = (unsigned long)va_arg(*ap, unsigned int);

  if(val == 0){
    *(str++) = '0';
    return;
  }
  while(val != 0){
    *(str++) = num[val % s->base];
    val /= s->base;
  }
  if((s->flags >> FMT_POUND) & 1u){
    if(s->base == 8)
      *(str++) = '0';
    else if(s->base == 16){
      if((s->flags >> FMT_UPPERCASE) & 1u)
        *(str++) = 'X';
      else
        *(str++) = 'x';
      *(str++) = '0';
    }
  }
  *str = '\0';
}

static int print_integer(fmt_info *s, char *buf, va_list *ap){
  char temp[30] = {0};
  if((s->flags >> FMT_SIGNED) & 1u)
    itoa(s, temp, ap);
  else
    utoa(s, temp, ap);  
  int temp_len = strlen_p(temp);
  for(int i = 0 ;i <= temp_len / 2 - 1 ;++i){
    char temp_char = temp[i];
    temp[i] = temp[temp_len - 1 - i];
    temp[temp_len - 1 - i] = temp_char;
  }
  if(s->width > temp_len)
    if((s->flags >> FMT_MINUS) & 1u){
      strcpy_p(buf, temp);
      buf += temp_len;
      for(int i = 1 ;i <= s->width - temp_len ;++i)
        pchar(buf++, ' ');
    } else {
      char padder = ((s->flags >> FMT_ZERO) & 1u) ? '0' : ' ';
      if(padder == '0'){
        if(temp[0] == '-' || temp[0] == '+' || temp[0] == ' ') // for signed
          pchar(buf++, temp[0]);
        if(temp[0] == '0' && (temp[1] == 'x' || temp[1] == 'X')){ // for unsigned
          pchar(buf++, temp[0]);
          pchar(buf++, temp[1]);
        }
        for(int i = 1 ;i <= s->width - temp_len ;++i)
          pchar(buf++, padder);
        if(temp[0] == '-' || temp[0] == '+' || temp[0] == ' ') 
          strcpy_p(buf, temp + 1);
        else if(temp[0] == '0' && (temp[1] == 'x' || temp[1] == 'X'))
          strcpy_p(buf, temp + 2);
        else 
          strcpy_p(buf, temp);
      } else {
        for(int i = 1 ;i <= s->width - temp_len ;++i)
          pchar(buf++, padder);
        strcpy_p(buf, temp);
      }
    }
  else
    strcpy_p(buf, temp);
  return s->width > temp_len ? s->width : temp_len;
}

static int print_float(fmt_info *s, char *buf, va_list *ap){
  return 0;
}

static int print_char(fmt_info *s, char *buf, va_list *ap){
  char ch = va_arg(*ap, int);
  if(s->width > 1)
    if(((s->flags >> FMT_MINUS) & 1u) == 1){
      pchar(buf++, ch);
      for(int i = 1 ;i <= s->width - 1 ;++i)
        pchar(buf++, ' ');
    } else {
      for(int i = 1 ;i <= s->width - 1 ;++i)
        pchar(buf++, ' ');
      pchar(buf++, ch);
    }
  else
    pchar(buf, ch);
  return s->width > 1 ? s->width : 1;
}

static int print_str(fmt_info *s, char *buf, va_list *ap){
  char *str = va_arg(*ap, char*);
  int str_length = strlen_p(str);
  if(s->width != 0 && str_length < s->width)
    if(((s->flags >> FMT_MINUS) & 1u) == 1){
      strcpy_p(buf, str);
      buf += str_length;
      for(int i = 1 ;i <= s->width - str_length ;++i)
        pchar(buf++, ' ');
    } else {
      for(int i = 1 ;i <= s->width - str_length ;++i)
        pchar(buf++, ' ');
      strcpy_p(buf, str);
    }
  else 
    strcpy_p(buf, str);
  return (str_length >= s->width) ? str_length : s->width;
}
//

int vsprintf(char*, const char*, va_list);
int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std_out = 1;
  int ret = vsprintf(NULL, fmt, ap);
  std_out = 0;
  return ret;
}

int vsprintf(char *out, const char *fmt, va_list args) {
  total_ch = 0;
  stophere = NULL;
  va_list ap;
  va_copy(ap, args);
  char *dout = out;
  while(*fmt != '\0'){
    if(*fmt != '%')
      pchar(dout, *fmt);
    else {
      fmt ++;
      if(*fmt == '%')
        pchar(dout, *fmt);
      else{
        fmt_info s = {0, 0, 0, 0};
        int ret = format_processor(&s, fmt, &ap);
        while(1){
          bool judge = 0;
          switch(*fmt){
            case 's': case 'c': case 'd': case 'i': case 'u': case 'o': case 'x': case 'X': case 'p':
            case 'f': case 'F': case 'a': case 'A': case 'e': case 'E': case 'g': case 'G':
              judge = 1; break;
            default: fmt ++;
          }
          if(judge) break;
        }
        int length = 0;
        switch(ret){
          case TYPE_INTEGER: length = print_integer(&s, dout, &ap); break;
          case TYPE_FLOAT: length = print_float(&s, dout, &ap); break;
          case TYPE_STR: length = print_str(&s, dout, &ap); break;
          case TYPE_CHAR: length = print_char(&s, dout, &ap); break;
          default: assert(0);
        }
        if(dout != NULL) dout += length - 1;
      }
    }
    if(dout != NULL) dout ++;
    fmt ++;
  }
  pchar(dout, '\0');
  return total_ch;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  return vsprintf(out, fmt, ap);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list args) {
  if(n <= 1){
    *out = '\0';
    return 0;
  }
  total_ch = 0;
  stophere = out + n - 1;
  va_list ap;
  va_copy(ap, args);
  char *dout = out;
  while(*fmt != '\0'){
    if(*fmt != '%')
      pchar(dout, *fmt);
    else {
      fmt ++;
      if(*fmt == '%')
        pchar(dout, *fmt);
      else{
        fmt_info s = {0, 0, 0, 0};
        int ret = format_processor(&s, fmt, &ap);
        while(1){
          bool judge = 0;
          switch(*fmt){
            case 's': case 'c': case 'd': case 'i': case 'u': case 'o': case 'x': case 'X':
            case 'f': case 'F': case 'a': case 'A': case 'e': case 'E': case 'g': case 'G':
              judge = 1; break;
            default: fmt ++;
          }
          if(judge) break;
        }
        int length = 0;
        switch(ret){
          
          case TYPE_INTEGER: length = print_integer(&s, dout, &ap); break;
          case TYPE_FLOAT: length = print_float(&s, dout, &ap); break;
          case TYPE_STR: length = print_str(&s, dout, &ap); break;
          case TYPE_CHAR: length = print_char(&s, dout, &ap); break;
          default:  assert(0);
        }
        dout += length - 1;
      }
    }
    dout ++;
    fmt ++;
  }
  pchar(dout, '\0');
  return total_ch;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(out, n, fmt, ap);
  return ret;
}


#endif
