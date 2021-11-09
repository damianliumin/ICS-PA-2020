#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t count = 0;
  while(*s != '\0'){
    count ++;
    s ++;
  }
  return count;
}

char *strcpy(char* dst,const char* src) {
  char *save = dst;
  while(*src != '\0'){
    *dst = *src;
    dst = dst + 1;
    src = src + 1;
  }
  *dst = *src;   // save '\0'
  return save;
}

char* strncpy(char* dst, const char* src, size_t n) {
  char *save = dst;
  int met_null = 0;
  for(int i = 1 ;i <= n ;++i){
    if(met_null){
      *dst = '\0';
      dst ++;
    } else {
      if(*src == '\0')
        met_null = 1;
      *dst = *src;
      dst ++;
      src ++;
    }
  }
  return save;
}

char* strcat(char* dst, const char* src) {
  char *save = dst;
  while(*dst != '\0')
    dst ++;
  while(*src != '\0'){
    *dst = *src;
    dst ++;
    src ++;
  }
  *dst = '\0';
  return save;
}

int strcmp(const char* s1, const char* s2) {
  while(1){
    if(*s1 - *s2 != 0)
      return *s1 - *s2;
    if(*s1 == '\0')  // equal and '\0'
      return 0;
    s1 ++;
    s2 ++;
  }
}

int strncmp(const char* s1, const char* s2, size_t n) {
  for(size_t i = 1 ;i <= n ;++i){
    if(*s1 - *s2 != 0)
      return *s1 - *s2;
    if(*s1 == '\0')  // equal and '\0'
      return 0;
    s1 ++;
    s2 ++;
  }
  return 0;
}

void* memset(void* v,int c,size_t n) {
  unsigned char* goal = v;
  unsigned char ch = c;
  for(size_t i = 0 ;i < n ;++i){
    *goal = ch;
    goal += 1;
  }
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  unsigned char *dout = out;
  const unsigned char *din = in;
  for(size_t i = 0 ;i < n ;++i){
    *dout = *din;
    dout = dout + 1;
    din = din + 1;
  }
  return out;
}

void* memmove(void* dst, const void* src, size_t n) {
  unsigned char *ddst = dst;
  const unsigned char *dsrc = src;
  if(dsrc < ddst && ddst < dsrc + n){
    ddst = ddst + n - 1;
    dsrc = dsrc + n - 1;
    for(size_t i = 0 ;i < n ;++i){
      *ddst = *dsrc;
      ddst -= 1;
      dsrc -= 1;
    }
  } else {
    for(size_t i = 0 ;i < n ;++i){
      *ddst = *dsrc;
      ddst += 1;
      dsrc += 1;
    }
  }
  return dst;
}

int memcmp(const void* s1, const void* s2, size_t n) {
  for(size_t i = 0 ;i < n ;++i){
    if(*(unsigned char*)s1 - *(unsigned char*)s2 != 0)
      return *(unsigned char*)s1 - *(unsigned char*)s2;
    s1 = (char*)s1 + 1;
    s2 = (char*)s2 + 1;
  }
  return 0;
}

#endif
