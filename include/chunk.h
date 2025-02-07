#ifndef CHUNK_H
#define CHUNK_H

#include "val.h"

typedef enum {
  OP_PUSH,          // push     rA B: loads the constant at index B in the constant pool and stores it in rA
  OP_PUSHLONG,      // pushlong rA B: loads the constant at index B in the constant pool and stores it in rA

  OP_TRUE,          // true     rA: loads true into rA
  OP_FALSE,         // false    rA: loads false into rA
  OP_NIL,           // nil      rA: loads nil into rA
  OP_ZERO,          // zero     rA: loads 0 into rA
  OP_ONE,           // one      rA: loads 1 into rA
  OP_MINUSONE,      // minusone rA: loads -1 into rA

  OP_NEG,           // neg      rA rB: negates rB and stores it in rA
  OP_NOT,           // not      rA rB: flips the boolean value of rB and stores it in rA
  OP_ISNIL,         // isnil    rA rB: checks if rB is nil and stores the result in rA
  OP_ISNOTNIL,      // isnotnil rA rB: checks if rB is not nil and stores the result in rA
  OP_ISZ,           // isz      rA rB: checks if rB is zero and stores the result in rA
  OP_SHOW,          // show     rA rB: convers rB to a string and stores it in rA

  OP_ADD,           // add      rA rB rC: adds rB to rC and stores the result in rA
  OP_SUB,           // sub      rA rB rC: subtracts rB to rC and stores the result in rA
  OP_MUL,           // mul      rA rB rC: multiplies rB by rC and stores the result in rA
  OP_DIV,           // div      rA rB rC: divides rB by rC and stores the result in rA
  OP_SHL,           // shl      rA rB rC: shifts left rB by rC bytes and stores the result in rA
  OP_SHR,           // shr      rA rB rC: shifts right rB by rC bytes and stores the result in rA
  OP_BAND,          // band     rA rB rC: performs binary AND on rB and rC and stores the result in rA
  OP_XOR,           // xor      rA rB rC: performs binary XOR on rB and rC and stores the result in rA
  OP_BOR,           // bor      rA rB rC: performs binary OR on rB and rC and stores the result in rA
  OP_NEQ,           // neq      rA rB rC: checks if rB and rC aren't equal and stores the result in rA
  OP_EQ,            // eq       rA rB rC: checks if rB and rC are equal and stores the result in rA
  OP_GT,            // gt       rA rB rC: checks if rB is greater than rC and stores the result in rA
  OP_LT,            // lt       rA rB rC: checks if rB is less than rC and stores the result in rA
  OP_GTE,           // gte      rA rB rC: checks if rB is greater or equal to rC and stores the result in rA
  OP_LTE,           // lte      rA rB rC: checks if rB is less than or equal to rC and stores the result in rA

  OP_CONCAT,        // concat   rA rB rC: concatenates rB and rC and stores the result in rA
  OP_UCONCAT,       // uconcat  rA rB rC: concatenates rB and rC and stores the result in rA

  OP_TABLENEW,      // tablenew rA: creates a new ObjTable and stores it in rA
  OP_TABLESET,      // tableset rA rB rC: sets the rB key in the rA table to rC
  OP_TABLEGET,      // tableget rA rB rC: retrieves the value associated with the rC key in the rB table and stores it in rA

  OP_RET,           // ret      rA: (right now) prints the value in rA and halts
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
