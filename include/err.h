#ifndef ERR_H
#define ERR_H

#include "bytecode.h"
#include "render.h"

typedef enum {
  ERR_CLI,
  ERR_LIST_OUT_OF_BOUNDS
} Err;

typedef struct {
  const char *fname;
  const char *src;

  RenderCtx ctx;
  int line;

  Err err;
} ErrMod;

bool runtimeErr(ErrMod *mod, Err id, Bytecode *bytecode, uint32_t offset);

void cliErr(const char *fmt, ...);

void reportErr(ErrMod mod, const char *fmt, ...);

void showOffendingLine(ErrMod mod);
void showNote(ErrMod mod, int line);
void showHint(ErrMod mod, const char *fmt, ...);

void freeErrMod(ErrMod *mod);

#endif
