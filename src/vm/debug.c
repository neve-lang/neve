#include <string.h>
#include <stdio.h>

#include "debug.h"
#include "val.h"

static void printOffset(size_t offset) {
  printf("\n%4zu  ", offset);
}

static size_t printReg(Chunk *ch, Val *regs, size_t offset) {
  const uint8_t reg = ch->code[offset];
  const Val val = regs[reg];

  printf("     r%u: ", reg);
  printVal(val);

  return offset + 1;
}

static size_t simpleInstr(const char *name, size_t offset) {
  printOffset(offset);
  printf("%-8s", name);

  return offset + 1;
}

static size_t constInstr(
  const char *name, 
  Chunk *ch, 
  Val *regs, 
  size_t offset
) {
  const uint8_t dest = ch->code[offset + 1];
  const uint8_t constOffset = ch->code[offset + 2];

  offset = printReg(ch, regs, offset + 1);

  printOffset(offset);
  printf("%-8s r%u  ", name, dest);
  printVal(ch->consts.consts[constOffset]);
  printf(" (%u)\n", constOffset);

  return offset + 1;
}

static size_t longConstInstr(
  const char *name, 
  Chunk *ch, 
  Val *regs,
  size_t offset
) {
  const uint8_t byteLength = 8;

  const uint32_t constOffset = (uint32_t)(
    ch->code[offset + 1] |
    (ch->code[offset + 2] << byteLength) |
    (ch->code[offset + 3] << byteLength * 2)
  );

  const uint8_t dest = ch->code[offset + 4];
  offset = printReg(ch, regs, offset + 4);

  printOffset(offset);
  printf("%-8s r%u", name, dest);
  printVal(ch->consts.consts[constOffset]);
  printf(" (%u)\n", constOffset);

  return offset + 1;
}

static size_t regInstr(const char *name, Chunk *ch, Val *regs, size_t offset) {
  const uint8_t reg = ch->code[offset + 1];

  offset = printReg(ch, regs, offset + 1);

  printOffset(offset);
  printf("%-8s r%u\n", name, reg);

  return offset + 2;
}

static size_t manyRegInstr(
  const char *name, 
  Chunk *ch, 
  Val *regs,
  size_t offset, 
  uint8_t regCount
) {
  // i'm sorry about this, but it's the easiest way to make this work
  int8_t regsInvolved[regCount];
  memset(regsInvolved, -1, regCount);
  
  size_t newOffset = offset + 1;
  for (uint8_t i = 0; i < regCount; i++) {
    const uint8_t reg = ch->code[newOffset];
    bool isAlreadyListed = false;

    for (uint8_t j = 0; j < regCount; j++) {
      if (regsInvolved[j] == (int8_t)reg) {
        isAlreadyListed = true;
      }
    }

    regsInvolved[i] = (int8_t)reg;


    if (isAlreadyListed) {
      newOffset++;
      continue;
    }

    newOffset = printReg(ch, regs, newOffset);
  }

  printOffset(offset);
  printf("%-8s ", name);

  for (uint8_t i = 0; i < regCount; i++) {
    uint8_t reg = ch->code[offset + i + 1];

    printf("r%u ", reg);
  }

  printf("\n");

  return offset + regCount + 1;
}

static size_t regByteInstr(
  const char *name,
  Chunk *ch,
  Val *regs,
  size_t offset
) {
  const uint8_t reg = ch->code[offset + 1];
  const uint8_t byte = ch->code[offset + 2]; 
  
  printReg(ch, regs, offset + 1);
  printOffset(offset);
  printf("%-8s r%u %u\n", name, reg, byte);

  return offset + 2;
}

void disasmChunk(Chunk *ch, Val *regs, const char *name) {
  printf("%s:\n", name);
  size_t offset = 0;

  while (offset < ch->next) {
    offset = disasmInstr(ch, regs, offset);
  }
}

size_t disasmInstr(Chunk *ch, Val *regs, size_t offset) {
  IGNORE(simpleInstr);
  IGNORE(regByteInstr);

  const uint8_t instr = ch->code[offset];

  switch (instr) {
    case OP_RET:
      return regInstr("ret", ch, regs, offset);

    case OP_CONST_LONG:
      return longConstInstr("pushl", ch, regs, offset);
    
    case OP_CONST:
      return constInstr("push", ch, regs, offset);

    case OP_TRUE:
      return regInstr("true", ch, regs, offset);

    case OP_FALSE:
      return regInstr("false", ch, regs, offset);

    case OP_NIL:
      return regInstr("nil", ch, regs, offset);

    case OP_ZERO:
      return regInstr("pushz", ch, regs, offset);

    case OP_MINUS_ONE:
      return regInstr("pushm1", ch, regs, offset);

    case OP_ONE:
      return regInstr("push1", ch, regs, offset);
    
    case OP_NEG:
      return manyRegInstr("neg", ch, regs, offset, 2);

    case OP_NOT:
      return manyRegInstr("not", ch, regs, offset, 2);

    case OP_IS_NIL:
      return manyRegInstr("isnil", ch, regs, offset, 2);

    case OP_IS_ZERO:
      return manyRegInstr("isz", ch, regs, offset, 2);

    case OP_SHOW:
      return manyRegInstr("show", ch, regs, offset, 2);

    case OP_ADD:
      return manyRegInstr("add", ch, regs, offset, 3);

    case OP_SUB:
      return manyRegInstr("sub", ch, regs, offset, 3);

    case OP_MUL:
      return manyRegInstr("mul", ch, regs, offset, 3);

    case OP_DIV:
      return manyRegInstr("div", ch, regs, offset, 3);

    case OP_CONCAT:
      return manyRegInstr("concat", ch, regs, offset, 3);

    case OP_UCONCAT:
      return manyRegInstr("uconcat", ch, regs, offset, 3);

    case OP_SHL:
      return manyRegInstr("shl", ch, regs, offset, 3);

    case OP_SHR:
      return manyRegInstr("shr", ch, regs, offset, 3);
    
    case OP_BIT_AND:
      return manyRegInstr("band", ch, regs, offset, 3);

    case OP_BIT_XOR:
      return manyRegInstr("xor", ch, regs, offset, 3);

    case OP_BIT_OR:
      return manyRegInstr("bor", ch, regs, offset, 3);

    case OP_EQ:
      return manyRegInstr("eq", ch, regs, offset, 3);

    case OP_NEQ:
      return manyRegInstr("neq", ch, regs, offset, 3);
    
    case OP_GREATER:
      return manyRegInstr("gt", ch, regs, offset, 3);

    case OP_LESS:
      return manyRegInstr("lt", ch, regs, offset, 3);

    case OP_GREATER_EQ:
      return manyRegInstr("gte", ch, regs, offset, 3);

    case OP_LESS_EQ:
      return manyRegInstr("lte", ch, regs, offset, 3);

    case OP_TABLE_NEW:
      return regInstr("tablenew", ch, regs, offset);

    case OP_TABLE_SET:
      return manyRegInstr("tableset", ch, regs, offset, 3);

    case OP_TABLE_GET:
      return manyRegInstr("tableget", ch, regs, offset, 3);

    default:
      printf("unknown instr %u\n", instr);
      return offset + 1;
  }

  return offset;
}
