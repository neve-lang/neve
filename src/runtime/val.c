#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "val.h"
#include "obj.h"

ValArr newValArr() {
  ValArr arr = {
    .cap = 0,
    .next = 0,
    .consts = NULL
  };

  return arr;
}

void writeValArr(ValArr *arr, Val val) {
  if (arr->next == arr->cap) {
    size_t oldCap = arr->cap; 

    arr->cap = GROW_CAP(oldCap);
    arr->consts = GROW_ARR(
      Val,
      arr->consts,
      oldCap,
      arr->cap
    );
  }

  arr->consts[arr->next++] = val;
}

void freeValArr(ValArr *arr) {
  FREE_ARR(Val, arr->consts, arr->cap);

  arr->cap = 0;
  arr->next = 0;
  arr->consts = NULL;
}

void printVal(Val val) {
  switch (val.type) {
    case VAL_BOOL:
      printf(VAL_AS_BOOL(val) ? "true" : "false");
      break;

    case VAL_NIL:
      printf("nil");
      break;
    
    case VAL_NUM:
      printf("%.14g", VAL_AS_NUM(val));
      break;

    case VAL_OBJ:
      printObj(val);
      break;
  }
}

bool valsEq(Val a, Val b) {
  if (a.type != b.type) {
    return false;
  }

  switch (a.type) {
    case VAL_NIL:
      return true;

    case VAL_BOOL:
      return VAL_AS_BOOL(a) && VAL_AS_BOOL(b);

    case VAL_NUM:
      return VAL_AS_NUM(a) == VAL_AS_NUM(b);

    case VAL_OBJ: {
      ObjStr *aStr = VAL_AS_STR(a);
      ObjStr *bStr = VAL_AS_STR(b);

      return (
        aStr->length == bStr->length &&
        memcmp(aStr->chars, bStr->chars, aStr->length) == 0
      );
    }
  }

  return false;
}

uint32_t valAsStr(char *buffer, Val val) {
  switch (val.type) {
    case VAL_OBJ:
      return objAsStr(buffer, VAL_AS_OBJ(val));

    case VAL_NIL: {
      const uint32_t length = 3;

      strncpy(buffer, "nil", length);
      return length;
    }

    case VAL_BOOL: {
      const bool isTrue = VAL_AS_BOOL(val);

      // we’re doing all this redundant stuff because clang-tidy doesn’t want
      // us to use magic values.
      // this is silly, but i also don’t want to disable the no-magic-values 
      // check.
      const uint32_t trueLength = 4;
      const uint32_t falseLength = 5;

      const uint32_t length = isTrue ? trueLength : falseLength;
      
      strncpy(buffer, isTrue ? "true" : "false", length);

      return length;
    }

    case VAL_NUM: {
      const uint32_t bufferSize = 32;

      const uint32_t length = (uint32_t)snprintf(
        buffer, 
        bufferSize, 
        "%.14g", 
        VAL_AS_NUM(val)
      ); 

      return length;
    }
  }

  return 0;
}
