#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>


static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;


static struct timeval boot;
static struct timeval current;
uint32_t NDL_GetTicks() {
  gettimeofday(&current, NULL);
  return (current.tv_sec - boot.tv_sec) * 1000 + 
    (current.tv_usec - boot.tv_usec + 500) / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", O_RDONLY, 0);
  return (read(fd, buf, len)) ? 1 : 0;
}

static int fb_w = 0, fb_h = 0;
static int cv_w = 0, cv_h = 0;
static int cv_x = 0, cv_y = 0;

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    if(write(fbctl, buf, len) == -1) assert(0);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  
  if(*w == 0 && *h == 0){
    *w = fb_w;
    *h = fb_h;
  }
  cv_w = *w;
  cv_h = *h;

  //assert(cv_w <= fb_w && cv_h <= fb_h);
  cv_x = (fb_w - cv_w) / 2;
  cv_y = (fb_h - cv_h) / 2;
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  if(x == 0 && y == 0 && w == 0 && h == 0){
    x = 0; y = 0;
    w = cv_w; h = cv_h;
  }
  int fd = open("/dev/fb", O_WRONLY, 0);
  for(int i = cv_y ;i < h + cv_y ;++i){
    lseek(fd, 4 * (x + cv_x + fb_w * (y + i)), SEEK_SET);
    if(write(fd, pixels, 4 * w) == -1) assert(0);
    pixels += w;
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  // set time
  gettimeofday(&boot, NULL);

  // get width and height of screen
  int fd = open("/proc/dispinfo", O_RDONLY, 0);
  char buf[64] = {0};
  if(read(fd, buf, sizeof(buf)) == -1) return 0;
  char *cur = buf;
  while((cur - buf < sizeof(buf)) && (*cur < '0' || *cur > '9'))
    ++cur;
  fb_w = atoi(cur);
  while((cur - buf < sizeof(buf)) && (*cur >= '0' && *cur <= '9'))
    ++cur;
  while((cur - buf < sizeof(buf)) && (*cur < '0' || *cur > '9'))
    ++cur;
  fb_h = atoi(cur);

  
  return 0;
}

void NDL_Quit() {
  boot.tv_sec = boot.tv_usec = 0;
}
