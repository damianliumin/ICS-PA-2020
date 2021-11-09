#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  char tmp[strlen(cmd) + 4] = {0};
  char *str = tmp, *filename = str;
  strcpy(str, cmd);
  if(str[strlen(str) - 1] == '\n')
    str[strlen(str) - 1] = '\0';
  //
  for(int i = 0 ;i < strlen(str) ;++i)
    if(str[i] == ' ')
      str[i] = '\0';

  char *argv[10] = {0};
  int argc = 0;
  if(strcmp(str, "setenv") == 0){
      str += strlen(str) + 1;
      int ret = setenv("PATH", str, 0);
      return;
  }
  argv[argc++] = filename;
  str += strlen(str) + 1;
  while(*str != 0){
    argv[argc++] = str;
    str += strlen(str) + 1;
  }
  execvp(filename, argv);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();
  setenv("PATH", "/bin:/usr/bin", 0);
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
