#ifndef CHUNK_H
#define CHUNK_H

#include "val.h"

typedef enum {
  OP_CONST,
  OP_CONST_LONG,
  OP_TRUE,
  OP_FALSE,
  OP_NIL,
  OP_ZERO,
  OP_ONE,
  OP_MINUS_ONE,
  OP_NEG,
  OP_NOT,
  OP_IS_NIL,
  OP_IS_NOT_NIL,
  OP_IS_ZERO,
  OP_SHOW,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_SHL,
  OP_SHR,
  OP_BIT_AND,
  OP_BIT_XOR,
  OP_BIT_OR,
  OP_NEQ,
  OP_EQ,
  OP_GREATER,
  OP_LESS,
  OP_GREATER_EQ,
  OP_LESS_EQ,
  OP_CONCAT,
  OP_RET,
} OpCode;

typedef struct {
  uint32_t offset;
  int line;
} Line;

typedef struct {
  uint32_t cap;
  uint32_t next;

  Line *lines;
} LineArr;

typedef struct {
  uint32_t cap;
  uint32_t next;

  uint8_t *code;
  ValArr consts;

  LineArr lines;
} Chunk;

Chunk newChunk();
void writeChunk(Chunk *ch, uint8_t byte, int line);
void freeChunk(Chunk *ch);
void writeConst(Chunk *ch, Val val, int line);
int addConst(Chunk *ch, Val val);

LineArr newLineArr();
void writeLineArr(LineArr *arr, int line, uint32_t offset);
void freeLineArr(LineArr *arr);

int getLine(Chunk *ch, uint32_t offset);

#endif
