#include <common.h>
#include <am.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  // yield();
  for(size_t i = 0 ;i < len ;++i)
    putch(*((char*)buf + i));
  return len;
}

#define NAMEINIT(key)  [ AM_KEY_##key ] = #key,
static const char *names[] = {
  AM_KEYS(NAMEINIT)
};
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);

extern int schedule_usr_current;

size_t events_read(void *buf, size_t offset, size_t len) {
  // yield();
  AM_INPUT_KEYBRD_T kbd;
  __am_input_keybrd(&kbd);
  if(kbd.keycode == AM_KEY_NONE) {
    *(char*)buf = 0;
    return 0;
  } else if(kbd.keycode == AM_KEY_F1) {
    schedule_usr_current = 1;
  } else if(kbd.keycode == AM_KEY_F2) {
    schedule_usr_current = 2;
  } else if(kbd.keycode == AM_KEY_F3) {
    schedule_usr_current = 3;
  }
  char temp[64] = {0};
  strcpy(temp, (kbd.keydown) ? "kd " : "ku ");
  strcpy(temp + 3, names[kbd.keycode]);
  temp[strlen(temp)] = '\n';
  strncpy((char*)buf, temp, len);
  return strlen(buf);
}

void __am_gpu_config(AM_GPU_CONFIG_T *);

static int fb_width = 0, fb_height = 0;

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T cfg;
  __am_gpu_config(&cfg);
  fb_width = cfg.width;
  fb_height = cfg.height;
  int ret = snprintf(buf, len, "WIDTH: %d\nHEIGHT: %d\n", fb_width, fb_height);
  return (len < ret) ? len : ret;
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl);

size_t fb_write(const void *buf, size_t offset, size_t len) {
  // yield();
  AM_GPU_FBDRAW_T ctl;
  ctl.sync = true;
  ctl.y = (offset / 4) / fb_width;
  ctl.x = (offset / 4) % fb_width;
  ctl.h = 1;
  ctl.w = len / 4;
  ctl.pixels = (char*)buf;
  __am_gpu_fbdraw(&ctl);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
