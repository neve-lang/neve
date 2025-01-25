#ifndef NEVE_CONST_H
#define NEVE_CONST_H

#define UNEXPECTED_BYTE 0

#include "common.h"
#include "vm.h"
#include "obj.h"

bool readConsts(
  NeveVM *vm, 
  ValArr *arr, 
  Bytecode *bytecode, 
  size_t *finalOffset
);

size_t readObj(
  NeveVM *vm,
  Val *into,
  size_t offset,
  Bytecode *bytecode
);

size_t readConst(
  NeveVM *vm,
  Val *into, 
  size_t offset, 
  Bytecode *bytecode
);


#endif
