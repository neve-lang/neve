#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "err.h"
#include "mem.h"
#include "obj.h"
#include "vm.h"

#ifdef DEBUG_EXEC
#include "debug.h"
#endif

#ifdef DEBUG_EXEC
static void printStack(NeveVM *vm) {
  printf("    ");

  for (Val *v = vm->regs; v < vm->top; v++) {
    printf("[");
    printVal(*v);
    printf("] ");
  } 

  printf("\n");
}
#endif

NeveVM newVM() {
  NeveVM vm = {
    .objs = NULL
  };

  initTable(&vm.strs, 0);

  return vm;
}

void freeVM(NeveVM *vm) {
  freeObjs(vm->objs);
  freeTable(&vm->strs);

  vm->objs = NULL;
}

void resetStack(NeveVM *vm) {
  vm->top = vm->regs;
}

#define READ_BYTE() (*vm->ip++)
#define READ_CONST() (vm->ch->consts.consts[READ_BYTE()])

static void concat(NeveVM *vm) {
  uint8_t regC = READ_BYTE();

  ObjStr *a = VAL_AS_STR(vm->regs[READ_BYTE()]);
  ObjStr *b = VAL_AS_STR(vm->regs[READ_BYTE()]);

  uint32_t length = a->length + b->length;

  char *chars = ALLOC(char, length + 1);

  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);

  chars[length] = '\0';

  uint32_t hash = hashStr(chars, length);

  const bool isInterned = length <= MAX_INTERNED_STR_SIZE;

  ObjStr *result = allocStr(vm, true, isInterned, chars, length, hash);
  vm->regs[regC] = OBJ_VAL(result);
}

static void uConcat(NeveVM *vm) {
  uint8_t regC = READ_BYTE();

  ObjUStr *a = VAL_AS_USTR(vm->regs[READ_BYTE()]);
  ObjUStr *b = VAL_AS_USTR(vm->regs[READ_BYTE()]);

  const Encoding encoding = a->encoding;

  uint32_t length = a->length + b->length;
  uint32_t byteLength = a->byteLength + b->byteLength;

  void *chars = ALLOC(char, byteLength + 1);

  memcpy(chars, a->chars, a->byteLength);
  memcpy((char *)chars + a->byteLength, b->chars, b->byteLength);

  memset((char *)chars + byteLength, '\0', 1);

  uint32_t hash = hashStr(chars, length);

  const bool isInterned = length <= MAX_INTERNED_STR_SIZE; 

  ObjUStr *result = allocUStr(
    vm,
    true,
    isInterned,
    encoding,
    chars,
    length,
    byteLength,
    hash
  );

  vm->regs[regC] = OBJ_VAL(result);
}

// NOLINTBEGIN
static Aftermath run(NeveVM *vm) {
#define BIN_OP(valType, op)                                                   \
  do {                                                                        \
    const uint8_t regC = READ_BYTE();                                         \
    const uint8_t regA = READ_BYTE();                                         \
    const uint8_t regB = READ_BYTE();                                         \
                                                                              \
    vm->regs[regC] = NUM_VAL(                                                 \
      VAL_AS_NUM(vm->regs[regA]) op VAL_AS_NUM(vm->regs[regB])                \
    );                                                                        \
  } while (false)
#define BIT_OP(op)                                                            \
  do {                                                                        \
    const uint8_t regC = READ_BYTE();                                         \
    const uint8_t regA = READ_BYTE();                                         \
    const uint8_t regB = READ_BYTE();                                         \
                                                                              \
    vm->regs[regC] = NUM_VAL(                                                 \
      (int)VAL_AS_NUM(vm->regs[regA]) op (int)VAL_AS_NUM(vm->regs[regB])      \
    );                                                                        \
  } while (false)

  while (true) {
#ifdef DEBUG_EXEC
    printStack(vm);

    const uint32_t offset = (uint32_t)(vm->ip - vm->ch->code);
    disasmInstr(vm->ch, vm->regs, offset);
#endif

    const uint8_t instr = READ_BYTE();

    switch (instr) {
      case OP_PUSH: {
        const uint8_t reg = READ_BYTE();

        vm->regs[reg] = READ_CONST();
        break;
      }
      
      case OP_PUSHLONG: {
        Chunk *ch = vm->ch;

#ifndef DEBUG_EXEC
        const uint32_t offset = (uint32_t)(vm->ip - vm->ch->code);
#endif

        const uint8_t byteLength = 8;
        const uint32_t constOffset = (uint32_t)(
          ch->code[offset + 1] |
          (ch->code[offset + 2] << byteLength) |
          (ch->code[offset + 3] << byteLength * 2)
        );

        const Val val = ch->consts.consts[constOffset]; 

        vm->regs[READ_BYTE()] = val;
        break;
      }

      case OP_TRUE:
        vm->regs[READ_BYTE()] = BOOL_VAL(true);
        break;

      case OP_FALSE:
        vm->regs[READ_BYTE()] = BOOL_VAL(false);
        break;

      case OP_NIL:
        vm->regs[READ_BYTE()] = NIL_VAL;
        break;

      case OP_ZERO:
        vm->regs[READ_BYTE()] = NUM_VAL(0);
        break;

      case OP_ONE:
        vm->regs[READ_BYTE()] = NUM_VAL(1);
        break;

      case OP_MINUSONE:
        vm->regs[READ_BYTE()] = NUM_VAL(-1);
        break;

      case OP_NEG:
        vm->regs[READ_BYTE()] = NUM_VAL(-VAL_AS_NUM(vm->regs[READ_BYTE()]));
        break;

      case OP_NOT:
        vm->regs[READ_BYTE()] = BOOL_VAL(!VAL_AS_BOOL(vm->regs[READ_BYTE()]));
        break;

      case OP_ISNIL:
        vm->regs[READ_BYTE()] = BOOL_VAL(IS_VAL_NIL(vm->regs[READ_BYTE()]));
        break;

      case OP_ISZ:
        vm->regs[READ_BYTE()] = BOOL_VAL(
          VAL_AS_NUM(vm->regs[READ_BYTE()]) == 0
        );

        break;

      case OP_SHOW: {
        const uint8_t destReg = READ_BYTE();
        const Val val = vm->regs[READ_BYTE()];

        const uint32_t size = valStrLength(val);
        char *buffer = ALLOC(char, size + 1);

        uint32_t finalSize = valAsStr(buffer, size, val);

        const bool isInterned = finalSize <= MAX_INTERNED_STR_SIZE;

        uint32_t hash = hashStr(buffer, finalSize);
        vm->regs[destReg] = OBJ_VAL(
          allocStr(vm, true, isInterned, buffer, finalSize, hash)
        );

        break;
      }

      case OP_ADD:
        BIN_OP(NUM_VAL, +);
        break;

      case OP_SUB:
        BIN_OP(NUM_VAL, -);
        break;

      case OP_MUL:
        BIN_OP(NUM_VAL, *);
        break;

      case OP_DIV:
        BIN_OP(NUM_VAL, /);
        break;

      case OP_CONCAT:
        concat(vm);
        break;

      case OP_UCONCAT:
        uConcat(vm);
        break;

      case OP_SHL:
        BIT_OP(<<);
        break;

      case OP_SHR:
        BIT_OP(>>);
        break;

      case OP_BAND:
        BIT_OP(&);
        break;

      case OP_XOR:
        BIT_OP(^);
        break;

      case OP_BOR:
        BIT_OP(|);
        break;

      case OP_EQ: {
        const uint8_t regC = READ_BYTE();
        const uint8_t regA = READ_BYTE();
        const uint8_t regB = READ_BYTE();

        vm->regs[regC] = BOOL_VAL(valsEq(
          vm->regs[regA], vm->regs[regB]
        ));

        break;
      }

      case OP_NEQ: {
        const uint8_t regC = READ_BYTE();
        const uint8_t regA = READ_BYTE();
        const uint8_t regB = READ_BYTE();

        vm->regs[regC] = BOOL_VAL(!valsEq(
          vm->regs[regA], vm->regs[regB]
        ));

        break;
      }

      case OP_GT:
        BIN_OP(BOOL_VAL, >);
        break;

      case OP_LT:
        BIN_OP(BOOL_VAL, <);
        break;

      case OP_GTE:
        BIN_OP(BOOL_VAL, >=);
        break;

      case OP_LTE:
        BIN_OP(BOOL_VAL, <=);
        break;

      case OP_TABLENEW: {
        vm->regs[READ_BYTE()] = (
          OBJ_VAL(newTable(vm, 0))
        );

        break;
      }

      case OP_TABLESET: {
        ObjTable *table = VAL_AS_TABLE(vm->regs[READ_BYTE()]);
        Val key = vm->regs[READ_BYTE()];

        tableSet(table->table, key, vm->regs[READ_BYTE()]);
        break;
      }

      case OP_RET: {
        Val val = vm->regs[READ_BYTE()];

        printVal(val);
        printf("\n");

        return AFTERMATH_OK;
      }

      default:
        // TODO: add an error message
        return AFTERMATH_RUNTIME_ERR;
    }
  }

#undef READ_BYTE
#undef READ_CONST
#undef BIN_OP
#undef BIT_OP
}
// NOLINTEND

Aftermath interpret(const char *fname, NeveVM *vm, Bytecode *bytecode) {
  Chunk ch = newChunk();

  if (!compile(vm, fname, bytecode, &ch)) {
    freeChunk(&ch); 

    return AFTERMATH_FILE_FORMAT_ERR;
  }

  vm->ch = &ch;
  vm->ip = ch.code;

  Aftermath aftermath = run(vm);

  if (aftermath != AFTERMATH_OK) {
    const uint32_t offset = (uint32_t)(vm->ip - vm->ch->code);

    ErrMod mod;
    bool failed = !runtimeErr(&mod, ERR_CLI, bytecode, offset);

    reportErr(mod, "runtime error");

    if (failed) {
      cliErr("couldn’t read source file");
    } else {
      showOffendingLine(mod);
    }

    freeErrMod(&mod);
  }

  freeChunk(&ch);

  IGNORE(run);
  return AFTERMATH_OK;
}
