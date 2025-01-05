#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "render.h"

// this is basically main.c:readFile() except it’s accomodated to return a
// null-terminated string.
static char *readSrc(const char *path) {
  FILE *f = fopen(path, "rb");

  if (f == NULL) {
    cliErr("source file %s not found", path);
    return NULL;
  }

  fseek(f, 0L, SEEK_END);
  size_t size = (size_t)ftell(f);
  rewind(f);

  char *buf = malloc(size + 1);
  if (buf == NULL) {
    cliErr("not enough memory available to read %s", path);
    return NULL;
  }

  size_t end = fread(buf, sizeof (char), size, f);

  if (end < size) {
    cliErr("%s: couldn't read the full source file", path);
    cliErr("this is most likely because a call to fread() failed.");

    return NULL;
  }

  buf[size] = '\0';

  fclose(f);
  return buf;
}

static int getLine(const uint8_t *bytes, uint32_t offset, uint32_t errOffset) {
  uint32_t newOffset = offset;

  int line = -1;
  uint32_t readOffset = 0;

  while (true) {
    memcpy(&readOffset, bytes + newOffset, sizeof (uint32_t));
    newOffset += sizeof (uint32_t);

    if (readOffset >= errOffset) {
      break;
    }

    memcpy(&line, bytes + newOffset, sizeof (uint32_t));
    newOffset += sizeof (uint32_t);
  }

  return line;
}

static bool readDebugHeader(
  ErrMod *mod, 
  Bytecode *bytecode, 
  uint32_t errOffset
) {
  bool successful = true;

  const uint8_t *bytes = bytecode->bytes;

  // +2: skip the length byte
  uint32_t offset = (uint32_t)bytecode->debugHeaderOffset + 2;
  
  uint16_t pathLength;
  memcpy(&pathLength, bytes + offset, sizeof (uint16_t));

  offset += sizeof (uint16_t);

  char *srcPath = malloc(pathLength + 1);
  memcpy(srcPath, bytes + offset, pathLength);

  srcPath[pathLength] = '\0';

  const char *src = readSrc(srcPath);
  if (src == NULL) {
    successful = false;
  }

  offset += pathLength;

  int line = getLine(bytes, offset, errOffset);

  RenderCtx ctx = newRenderCtx(line);

  mod->fname = srcPath;
  mod->src = src;
  mod->ctx = ctx;
  mod->line = line;

  return successful;
}

bool runtimeErr(ErrMod *mod, Err id, Bytecode *bytecode, uint32_t offset) {
  mod->err = id;

  if (!readDebugHeader(mod, bytecode, offset)) {
     return false;
  }

  return true;
}

void cliErr(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  renderErrMsg(ERR_CLI, fmt, args);
  va_end(args);
}

void reportErr(ErrMod mod, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);

  renderErrMsg(mod.err, fmt, args); 
  renderLocus(mod.ctx, mod.fname); 

  va_end(args);
}

void showOffendingLine(ErrMod mod) {
  renderLine(mod.ctx, mod.src);
}

void showNote(ErrMod mod, int line) {
  mod.ctx.line = line;

  renderRegularLine(mod.ctx, mod.src);
}

void showHint(ErrMod mod, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);

  renderHint(mod.ctx, fmt, args);

  va_end(args);
}

void freeErrMod(ErrMod *mod) {
  free((char *)mod->fname);
  free((char *)mod->src);
}
