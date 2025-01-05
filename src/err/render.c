#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"

static int digitsIn(int n) {
  return (int)floor(log10(abs(n)) + 1);
}

static void write(const char *s) {
  fprintf(stderr, "%s", s);
}

static void writef(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

static void endFormat() {
  write(RESET "\n");
}

static const char *findLine(const char *src, int line) {
  if (src == NULL || line < 1) {
    return NULL;
  }

  int currLine = 1; 
  const char *curr = src;

  while (*curr != '\0' && currLine < line) {
    if (*curr == '\n') {
      currLine++;
    }

    curr++;
  }

  if (currLine < line) {
    return NULL;
  }

  return curr;
}

static void writeFrom(const char *src, const int until) {
  int col = 1;
  const char *curr = src;

  while (col <= until && *curr != '\n') {
    fputc(*curr, stderr); 
    curr++;
    col++;
  }
}

static void writeEmptyPipe(const int lineDigits) {
  writef(BLUE "%*s | \n", lineDigits, "");
}

static void writeLinePipes(const int lineDigits, const int line) {
  writeEmptyPipe(lineDigits);
  writef("%*d | " RESET, lineDigits, line); 
}

RenderCtx newRenderCtx(int line) {
  RenderCtx ctx = {
    .line = line,
    // +1 in case we want to render a line below.
    .lineDigits = digitsIn(line + 1)
  };

  return ctx;
}

void renderErrMsg(int id, const char *fmt, va_list args) {
  write(RED "error" WHITE ": ");

  vfprintf(stderr, fmt, args);

  if (id < 1) {
    endFormat();
    return;
  }

  writef(RED " [RE%03d]", id);

  endFormat();
}

void renderLocus(RenderCtx ctx, const char *fname) {
  write(BLUE "   in" WHITE ": ");

  writef("%s:%d", fname, ctx.line);

  endFormat();
}

void renderLine(RenderCtx ctx, const char *src) {
  const int line = ctx.line;
  const char *lineStart = findLine(src, line);
  const int lineEnd = (int)strcspn(lineStart, "\n");

  writeLinePipes(ctx.lineDigits, line);

  if (lineStart == NULL) {
    write(RED "could not find line");
  }

  writeFrom(lineStart, lineEnd);

  endFormat();
}

void renderHint(RenderCtx ctx, const char *fmt, va_list args) {
  writef(BLUE "%*s -> ", ctx.lineDigits, "");

  vfprintf(stderr, fmt, args);

  endFormat();
}

void renderFmtLine(RenderCtx ctx, const char *fmt, va_list args) {
  writeLinePipes(ctx.lineDigits, ctx.line);

  vfprintf(stderr, fmt, args);

  endFormat();
  writeEmptyPipe(ctx.lineDigits);
}

void renderRegularLine(RenderCtx ctx, const char *src) {
  const int line = ctx.line;
  const char *lineStart = findLine(src, line);
  const int lineEnd = (int)strcspn(lineStart, "\n");

  writeLinePipes(ctx.lineDigits, ctx.line);

  if (lineStart == NULL) {
    write(RED "could not find line");
  }

  writeFrom(lineStart, lineEnd);

  endFormat();
}

