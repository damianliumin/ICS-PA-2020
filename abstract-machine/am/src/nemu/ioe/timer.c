#include <am.h>
#include <nemu.h>

static uint32_t seconds, useconds;
static uint32_t boot_seconds, boot_useconds;

void __am_timer_init() {
  boot_useconds = inl(RTC_ADDR);
  boot_seconds = inl(RTC_ADDR + 4);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  useconds = inl(RTC_ADDR) - boot_useconds;
  seconds = inl(RTC_ADDR + 4) - boot_seconds;
  uptime->us = seconds * 1000000 + (useconds + 500);
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
