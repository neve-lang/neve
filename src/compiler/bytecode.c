#include <string.h>

#include "bytecode.h"

static bool isBytecodeTruncated(Bytecode *bytecode) {
  const uint8_t *bytes = bytecode->bytes;
  const size_t length = bytecode->length;

  if (length < EOF_PADDING_SIZE) {
    return true;
  }

  return memcmp(
    bytes + length - EOF_PADDING_SIZE, 
    EOF_PADDING, 
    EOF_PADDING_SIZE
  ) != 0;
}

static bool checkMagicNumber(const uint8_t *bytes) {
  uint32_t firstFourBytes;
  memcpy(&firstFourBytes, bytes, sizeof (uint32_t));

  return firstFourBytes == NEVE_MAGIC_NUMBER;
}

Bytecode newBytecode(const uint8_t *bytes, size_t length) {
  Bytecode bytecode = {
    .bytes = bytes,
    .length = length
  };

  return bytecode;
}

bool isValidBytecode(Bytecode *bytecode) {
  return !isBytecodeTruncated(bytecode) && checkMagicNumber(bytecode->bytes);
}

