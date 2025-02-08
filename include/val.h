#ifndef VAL_H
#define VAL_H

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjStr ObjStr;
typedef struct ObjUStr ObjUStr;
typedef struct ObjTable ObjTable;

#define BOOL_VAL(val) ((Val){ VAL_BOOL, {.boolean = (val) } })
#define NIL_VAL       ((Val){ VAL_NIL, { .num = 0 } })
#define NUM_VAL(val)  ((Val){ VAL_NUM, { .num = (val) } })
#define OBJ_VAL(val)  ((Val){ VAL_OBJ, { .obj = (Obj *)(val) } })
#define EMPTY_VAL     ((Val){ VAL_EMPTY, { .num = 0 } })

#define IS_VAL_BOOL(val)  ((val).type == VAL_BOOL)
#define IS_VAL_NIL(val)   ((val).type == VAL_NIL)
#define IS_VAL_NUM(val)   ((val).type == VAL_NUM)
#define IS_VAL_OBJ(val)   ((val).type == VAL_OBJ)
#define IS_VAL_EMPTY(val) ((val).type == VAL_EMPTY)

#define VAL_AS_BOOL(val)    ((val).as.boolean)
#define VAL_AS_NUM(val)     ((val).as.num)
#define VAL_AS_OBJ(val)     ((val).as.obj)

typedef enum {
  VAL_NUM,
  VAL_BOOL,
  VAL_NIL,
  VAL_OBJ,

  // empty table buckets
  VAL_EMPTY
} ValType;

typedef struct {
  ValType type;

  union {
    bool boolean;
    double num;
    Obj *obj;
  } as;
} Val;

typedef struct {
  size_t cap; 
  size_t next;

  Val *consts;
} ValArr;

ValArr newValArr();
void writeValArr(ValArr *arr, Val val);
void freeValArr(ValArr *arr);
void printVal(Val val);

uint32_t hashVal(Val val);

bool valsEq(Val a, Val b);

uint32_t valStrLength(Val val);
uint32_t valAsStr(char *buffer, uint32_t size, Val val);

#endif
