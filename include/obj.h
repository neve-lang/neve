#ifndef OBJ_H
#define OBJ_H

#include "val.h"
#include "vm.h"

#define OBJ_TYPE(val)     (VAL_AS_OBJ(val)->type)

#define VAL_AS_STR(val)   ((ObjStr *)VAL_AS_OBJ(val))
#define VAL_AS_CSTR(val)  (((ObjStr *)VAL_AS_OBJ(val))->chars)

#define VAL_AS_TABLE(val) ((ObjTable *)VAL_AS_OBJ(val))

typedef enum {
  OBJ_STR,
  OBJ_TABLE
} ObjType;

struct Obj {
  ObjType type;

  struct Obj *next;
};

struct ObjStr {
  Obj obj; 

  bool ownsStr;
  uint32_t length;
  const char *chars;

  uint32_t hash;
};

struct ObjTable {
  Obj obj;

  Table *table;
};

/*
we don’t need this function thanks to type checking, but if it
ends up being absolutely necessary...  let’s just keep it in
here for now.
static inline bool isObjType(Val val, ObjType type) {
  return IS_VAL_OBJ(val) && OBJ_TYPE(val) == type;
}
*/

ObjStr *allocStr(
  NeveVM *vm,
  bool ownsStr,
  const char *chars,
  uint32_t length,
  uint32_t hash
);

ObjTable *allocTable(NeveVM *vm);

uint32_t hashStr(const char *key, uint32_t length);

void printObj(Val val);

void freeObj(Obj *obj);

uint32_t objStrLength(Obj *obj);
uint32_t objAsStr(const char *buffer, Obj *obj);

#endif
