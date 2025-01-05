#include "bytecode.h"

Bytecode newBytecode(const uint8_t *bytes, size_t length) {
  Bytecode bytecode = {
    .bytes = bytes,
    .length = length
  };

  return bytecode;
}
