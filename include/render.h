#ifndef RENDER_H
#define RENDER_H

#include <stdarg.h>

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define WHITE "\033[1;37m"
#define RESET "\033[0m"

typedef struct {
  int lineDigits;
  int line;
} RenderCtx;

RenderCtx newRenderCtx(int line);

void renderErrMsg(int id, const char *fmt, va_list args);
void renderLocus(RenderCtx ctx, const char *fname);
void renderLine(RenderCtx ctx, const char *src);
void renderHint(RenderCtx ctx, const char *fmt, va_list args);
void renderFmtLine(RenderCtx ctx, const char *fmt, va_list args);
void renderRegularLine(RenderCtx ctx, const char *src);

#endif
