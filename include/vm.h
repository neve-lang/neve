#ifndef VM_H
#define VM_H

#include "bytecode.h"
#include "chunk.h"
#include "table.h"
#include "val.h"

#define STACK_MAX 256

typedef struct {
  Chunk *ch;
  uint8_t *ip;

  Val regs[STACK_MAX];
  Val *top;

  Table strs;
  Obj *objs;
} NeveVM;

typedef enum {
  AFTERMATH_OK,
  AFTERMATH_FILE_FORMAT_ERR,
  AFTERMATH_RUNTIME_ERR
} Aftermath;

NeveVM newVM();
void freeVM(NeveVM *vm);

void resetStack(NeveVM *vm);

Aftermath interpret(const char *fname, NeveVM *vm, Bytecode *bytecode);

#endif
