#include <am.h>
#include <nemu.h>
#define SYNC_ADDR (VGACTL_ADDR + 4)

static int width, height;

void __am_gpu_init() {
  width = inw(VGACTL_ADDR + 2);
  height = inw(VGACTL_ADDR);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = width * height * sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;
  int cp_dword = (w <= width - x) ? w : width - x;
  int hmax = (height < y + h) ? height : y + h;
  int i = 0, j = 0;
  for(j = y ;j < hmax ;++j) {
    int start = width * j + x;
    for(i = 0 ;i < cp_dword ;++i){
      uint32_t *fb = (uint32_t*)(uintptr_t)(FB_ADDR + sizeof(uint32_t) * (start + i));
      *fb = *(pixels++);
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
