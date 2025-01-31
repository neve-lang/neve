#include <stdio.h>
#include <string.h>

#include "const.h"
#include "obj.h"

#define READ(into, bytes, offset, type)                 \
  do {                                                  \
    memcpy(&(into), (bytes) + (offset), sizeof (type)); \
    (offset) += sizeof (type);                          \
  } while (false)

static size_t readStr(
  NeveVM *vm, 
  Val *into, 
  size_t offset, 
  Bytecode *bytecode
) {
  const uint8_t *bytes = bytecode->bytes;
  size_t newOffset = offset;

  uint32_t length;
  READ(length, bytes, newOffset, uint32_t);

  const size_t strEndOffset = newOffset + (size_t)length;
  if (strEndOffset >= bytecode->length - EOF_PADDING_SIZE) {
    return UNEXPECTED_BYTE;
  }

  char *chars = (char *)(bytes + newOffset);

  newOffset += length;

  uint32_t hash = hashStr(chars, length);
  
  const bool isInterned = bytes[newOffset++];

  ObjStr *str = allocStr(vm, false, isInterned, chars, length, hash);
  *into = OBJ_VAL(str);

  return newOffset;
}

static size_t readTable(
  NeveVM *vm,
  Val *into,
  size_t offset,
  Bytecode *bytecode
) {
  const uint8_t *bytes = bytecode->bytes;
  size_t newOffset = offset;

  uint32_t tableSize;
  READ(tableSize, bytes, newOffset, uint32_t);

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

  return newOffset;
}

static size_t readUStr(
  NeveVM *vm,
  Val *into,
  size_t offset,
  Bytecode *bytecode
) {
  const uint8_t *bytes = bytecode->bytes;
  size_t newOffset = offset;

  const uint8_t byte = bytes[newOffset++];
  const Encoding encoding = (Encoding)byte;

  uint32_t length;
  READ(length, bytes, newOffset, uint32_t);

  uint32_t byteLength;
  READ(byteLength, bytes, newOffset, uint32_t);

  const size_t strEndOffset = newOffset + (size_t)byteLength;
  if (strEndOffset >= bytecode->length - EOF_PADDING_SIZE) {
    return UNEXPECTED_BYTE;
  }

  void *contents = (void *)(bytes + newOffset);  
  newOffset += byteLength;

  // i'm not sure whether this is safe?
  const char *key = (char *)contents;
  const uint32_t hash = hashStr(key, byteLength);

  const bool isInterned = (bool)bytes[newOffset++]; 

  ObjUStr *str = allocUStr(
    vm, 
    false, 
    isInterned, 
    encoding, 
    contents, 
    length, 
    byteLength, 
    hash
  );

  *into = OBJ_VAL(str);  

  return newOffset;
}

size_t readObj(
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
    case OBJ_STR:
      newOffset = readStr(vm, into, newOffset, bytecode);
      break;

    case OBJ_USTR:
      newOffset = readUStr(vm, into, newOffset, bytecode);
      break;

    case OBJ_TABLE:
      newOffset = readTable(vm, into, newOffset, bytecode);
      break;

    default:
      return UNEXPECTED_BYTE;
  }

  return newOffset;
}

size_t readConst(
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
      uint8_t b = bytes[newOffset++];

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
      READ(n, bytes, newOffset, double);

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

bool readConsts(
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


