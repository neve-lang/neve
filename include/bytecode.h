#ifndef NEVE_BYTECODE_H
#define NEVE_BYTECODE_H

#include "common.h"

typedef struct {
  const uint8_t *bytes;
  const size_t length;

  size_t debugHeaderOffset;
} Bytecode;

Bytecode newBytecode(const uint8_t *bytes, size_t length);

bool isValidBytecode(Bytecode *bytecode);

#endif
