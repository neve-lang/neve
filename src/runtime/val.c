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

    case VAL_EMPTY:
      printf("()");
      break;
  }
}

bool valsEq(Val a, Val b) {
  switch (a.type) {
    case VAL_NIL:
      return true;

    case VAL_BOOL:
      return VAL_AS_BOOL(a) && VAL_AS_BOOL(b);

    case VAL_NUM:
      return VAL_AS_NUM(a) == VAL_AS_NUM(b);

    case VAL_OBJ:
      return objsEq(VAL_AS_OBJ(a), VAL_AS_OBJ(b));

    case VAL_EMPTY:
      return true;
  }

  return false;
}

static uint32_t hashDouble(double val) {
  union BitCast {
    double val;
    uint32_t ints[2];
  };

  union BitCast cast;
  cast.val = val + 1;

  return cast.ints[0] + cast.ints[1];
}

// NOLINTBEGIN
uint32_t hashVal(Val val) {
  switch (val.type) {
    case VAL_BOOL:
      return VAL_AS_BOOL(val) ? 3 : 5;

    case VAL_NIL:
      return 7;
  
    case VAL_NUM:
      return hashDouble(VAL_AS_NUM(val));

    case VAL_OBJ:
      return VAL_AS_STR(val)->hash;

    case VAL_EMPTY:
      return 0;
  }

  return 0;
}
// NOLINTEND

uint32_t valStrLength(Val val) {
  switch (val.type) {
    case VAL_OBJ:
      return objStrLength(VAL_AS_OBJ(val));

    case VAL_NIL:
      return 3;

    case VAL_BOOL:
      return 4 + !VAL_AS_BOOL(val);

    case VAL_NUM:
      return (uint32_t)snprintf(NULL, 0, "%.14g", VAL_AS_NUM(val));

    case VAL_EMPTY:
      return 2;
  }

  return 0;
}

uint32_t valAsStr(char *buffer, const uint32_t size, Val val) {
  switch (val.type) {
    case VAL_OBJ:
      return objAsStr(buffer, size, VAL_AS_OBJ(val));

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

    case VAL_EMPTY: {
      const uint32_t length = 2;

      strncpy(buffer, "()", length); 

      return length;
    }
  }

  return 0;
}
