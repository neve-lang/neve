#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "const.h"
#include "err.h"

static size_t skipDebugHeader(const uint8_t *bytes, size_t offset) {
  uint16_t headerLength;
  memcpy(&headerLength, bytes + offset, sizeof (uint16_t));

  const size_t opcodeOffset = offset + sizeof (uint16_t) + headerLength;  
  if (bytes[opcodeOffset - 1] != NEVE_CONST_HEADER_SEPARATOR) {
    return UNEXPECTED_BYTE;
  }

  return opcodeOffset;
}

bool compile(NeveVM *vm, const char *fname, Bytecode *bytecode, Chunk *ch) {
  if (!isValidBytecode(bytecode)) {
    cliErr("%s: unexpected file format", fname);
    cliErr("the bytecode file either contains invalid bytecode or");
    cliErr("it may have been truncated");

    return false;
  }

  ValArr *consts = &ch->consts;
  size_t offset;

  if (!readConsts(vm, consts, bytecode, &offset)) {
    cliErr("%s: failed to load constants", fname);
    
    freeValArr(consts);
    return false;
  }

  const size_t debugHeaderOffset = offset;
  bytecode->debugHeaderOffset = debugHeaderOffset;

  offset = skipDebugHeader(bytecode->bytes, offset);

  if (offset == UNEXPECTED_BYTE) {
    cliErr("%s: failed to load file information", fname);

    freeValArr(consts);
    return false;
  }

  ch->code = (uint8_t *)(bytecode->bytes + offset);
  ch->next = (uint32_t)(bytecode->length - offset - EOF_PADDING_SIZE);

  return true;
}
#undef UNEXPECTED_BYTE
