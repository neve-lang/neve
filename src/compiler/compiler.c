#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "err.h"
#include "obj.h"

#define UNEXPECTED_BYTE 0

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

static bool isValidBytecode(Bytecode *bytecode) {
  return !isBytecodeTruncated(bytecode) && checkMagicNumber(bytecode->bytes);
}

static size_t skipDebugHeader(const uint8_t *bytes, size_t offset) {
  uint16_t headerLength;
  memcpy(&headerLength, bytes + offset, sizeof (uint16_t));

  const size_t opcodeOffset = offset + sizeof (uint16_t) + headerLength;  
  if (bytes[opcodeOffset - 1] != NEVE_CONST_HEADER_SEPARATOR) {
    return UNEXPECTED_BYTE;
  }

  return opcodeOffset;
}

static size_t readObj(
  NeveVM *vm,
  Val *into,
  size_t offset,
  Bytecode *bytecode
);

static size_t readConst(
  NeveVM *vm,
  Val *into, 
  size_t offset, 
  Bytecode *bytecode
);

static size_t readObj(
  NeveVM *vm,
  Val *into, 
  size_t offset, 
  Bytecode *bytecode
) {
  size_t newOffset = offset;

  const uint8_t *bytes = bytecode->bytes;

  uint8_t byte = bytes[newOffset++]; 
  ObjType type = (ObjType)byte;

  switch (type) {
    case OBJ_STR: {
      uint32_t length;
      memcpy(&length, bytes + newOffset, sizeof (uint32_t));

      newOffset += sizeof (uint32_t);

      const size_t strEndOffset = newOffset + (size_t)length;
      if (strEndOffset >= bytecode->length - EOF_PADDING_SIZE) {
        return UNEXPECTED_BYTE;
      }

      char *chars = (char *)(bytes + newOffset);

      newOffset += length;

      uint32_t hash = hashStr(chars, length);

      ObjStr *str = allocStr(vm, false, chars, length, hash);
      *into = OBJ_VAL(str);

      break;
    }

    case OBJ_TABLE: {
      uint32_t tableSize;
      memcpy(&tableSize, bytes + newOffset, sizeof (uint32_t));

      newOffset += sizeof (uint32_t);

      ObjTable *obj = newTable(vm, tableSize);
      Table *table = obj->table;

      // TODO: determine whether we should keep things this way
      // or avoid the overhead of tableSet(), even if this overhead
      // only affects startup time.
      for (uint32_t i = 0; i < tableSize; i++) {
        Val key;
        newOffset = readConst(vm, &key, newOffset, bytecode);

        if (newOffset == UNEXPECTED_BYTE) {
          return UNEXPECTED_BYTE; 
        }

        Val val;
        newOffset = readConst(vm, &val, newOffset, bytecode);

        if (newOffset == UNEXPECTED_BYTE) {
          return UNEXPECTED_BYTE;
        }

        tableSet(table, key, val);
      }

      *into = OBJ_VAL(obj);

      break;
    }

    default:
      return UNEXPECTED_BYTE;
  }

  return newOffset;
}

static size_t readConst(
  NeveVM *vm,
  Val *into, 
  size_t offset, 
  Bytecode *bytecode
) {
  size_t newOffset = offset;

  const uint8_t *bytes = bytecode->bytes;

  uint8_t byte = bytes[newOffset++];
  ValType type = (ValType)byte;

  switch (type) {
    case VAL_BOOL: {
      uint8_t b;
      memcpy(&b, bytes + newOffset, sizeof (uint8_t));

      newOffset += sizeof (uint8_t);

      *into = BOOL_VAL(b);
      break;
    }

    case VAL_NIL:
      *into = NIL_VAL;
      break;

    case VAL_EMPTY:
      *into = EMPTY_VAL;
      break;

    case VAL_NUM: {
      double n;
      memcpy(&n, bytes + newOffset, sizeof (double));

      newOffset += sizeof (double);

      *into = NUM_VAL(n);
      break;
    }

    case VAL_OBJ: {
      newOffset = readObj(vm, into, newOffset, bytecode);

      if (newOffset == UNEXPECTED_BYTE) {
        return UNEXPECTED_BYTE;
      }

      break;
    }

    default:
      return UNEXPECTED_BYTE; 
  }

  return newOffset;
}

static bool readConsts(
  NeveVM *vm, 
  ValArr *arr, 
  Bytecode *bytecode, 
  size_t *finalOffset
) {
  const uint8_t *bytes = bytecode->bytes;
  size_t offset = sizeof (uint32_t);

  while (true) {
    uint8_t byte = bytes[offset];
    if (byte == NEVE_CONST_HEADER_SEPARATOR) {
      offset++;

      break;
    } 

    if (byte == EOF_PADDING_BYTE) {
      return false;
    }

    Val into;
    offset = readConst(vm, &into, offset, bytecode);

    if (offset == UNEXPECTED_BYTE) {
      return false;
    }

    writeValArr(arr, into);

  }

  *finalOffset = offset;
  return true;
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
