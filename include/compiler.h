#ifndef COMPILER_H
#define COMPILER_H

#include "chunk.h"
#include "vm.h"

bool compile(NeveVM *vm, const char *fname, Bytecode *bytecode, Chunk *ch);

#endif
